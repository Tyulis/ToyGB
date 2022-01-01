#include "audio/mapping/AudioNoiseMapping.hpp"

#define OFFSET_START IO_CH4_LENGTH
#define OFFSET_LENGTH   IO_CH4_LENGTH - OFFSET_START
#define OFFSET_ENVELOPE IO_CH4_ENVELOPE - OFFSET_START
#define OFFSET_COUNTER  IO_CH4_COUNTER - OFFSET_START
#define OFFSET_CONTROL  IO_CH4_CONTROL - OFFSET_START

/** Noise channel control IO registers mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF20 |       0000 | NR41 | --WWWWWW | Sound length counter (bit 0-5)
      FF21 |       0001 | NR42 | BBBBBBBB | Volume envelope control : VVVVDPPP
           |            |      |          | - V (bit 4-7) : Initial volume envelope (0-15)
           |            |      |          | - D (bit 3) : Envelope direction (0 = decrease, 1 = increase)
           |            |      |          | - P (bit 0-2) : Envelope update period (0-7)
      FF22 |       0002 | NR43 | BBBBBBBB | Noise generation control : SSSSWPPP
           |            |      |          | - S (bit 4-7) : Noise generation period shift
           |            |      |          | - W (bit 3) : LFSR used width (0 = 15 bits, 1 = 7 bits)
		   |            |      |          | - P (bit 0-2) : Base generation period value (the period in APU cycles is NOISE_PERIOD_BASES[P] << S)
           |            |      |          | The digital-to-analog circuit is controlled by bits 3-7 of NR22, and disables the channel if they are all zero
      FF23 |       0003 | NR44 | WB------ | Channel enable control : SL------
           |            |      |          | - S (bit 7) : Trigger the channel operation (Set to 1 to start)
           |            |      |          | - L (bit 6) : Enable the length counter (0 = disabled, 1 = counting, stops the channel when it reaches zero) */


namespace toygb {
	// Base noise generation periods (the index is NR43.0-2)
	const int NOISE_PERIOD_BASES[] = {4, 8, 16, 24, 32, 40, 48, 56};

	// Initialize the channel.
	AudioNoiseMapping::AudioNoiseMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, HardwareConfig* hardware) : AudioChannelMapping(channel, control, debug, hardware) {
		length = 0x3F;

		initialEnvelopeVolume = 0;
		envelopeDirection = false;
		envelopePeriod = 0;

		periodShift = 0;
		registerWidth = false;
		periodBase = 0;

		enableLength = false;

		m_baseTimerCounter = 0;
		m_envelopeFrameCounter = 0;
	}

	// Get the value at the given relative address
	uint8_t AudioNoiseMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_LENGTH:  // NR41
				return 0xFF;
			case OFFSET_ENVELOPE:  // NR42
				return (
					(initialEnvelopeVolume << 4) |
					(envelopeDirection << 3) |
					envelopePeriod);
			case OFFSET_COUNTER:  // NR43
				return (
					(periodShift << 4) |
					(registerWidth << 3) |
					periodBase);
			case OFFSET_CONTROL:  // NR44
				return (enableLength << 6) | 0xBF;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	// Set the value at the given relative address
	void AudioNoiseMapping::set(uint16_t address, uint8_t value) {
		// Ignore writes when the APU is powered off
		// On DMG hardware, length registers are still fully writable even with the APU powered off
		if (powered || (m_hardware->isDMGConsole() && address == OFFSET_LENGTH)) {
			switch (address) {
				case OFFSET_LENGTH:  // NR41
					length = 64 - (value & 0x3F);
					break;
				case OFFSET_ENVELOPE:  // NR42
					// The higher 5 bits control the DAC : if they are all 0, the channel is disabled
					if ((value & 0xF8) == 0)
						disable();
					initialEnvelopeVolume = (value >> 4) & 0x0F;
					envelopeDirection = (value >> 3) & 1;
					envelopePeriod = value & 7;
					break;
				case OFFSET_COUNTER:  // NR43
					periodShift = (value >> 4) & 0x0F;
					registerWidth = (value >> 3) & 1;
					periodBase = value & 7;
					break;
				case OFFSET_CONTROL: {  // NR44
					bool wasEnabled = enableLength;
					enableLength = (value >> 6) & 1;

					// Enabling length counter in first half of the length period (next frame does not clock length) clocks length once
					if (!wasEnabled && enableLength && m_frameSequencer % 2 == 0 && length > 0)
						onLengthFrame();

					if ((value >> 7) & 1)
						reset();
					break;
				}
			}
		}
	}

	// Called every APU cycle (= 2 clocks)
	void AudioNoiseMapping::onUpdate() {
		m_baseTimerCounter += 1;
		// Period in APU cycles is NOISE_PERIOD_BASES[periodBase] << periodShift (*2 for clocks)
		if (m_baseTimerCounter >= NOISE_PERIOD_BASES[periodBase] << periodShift) {
			// Emulate linear feedback shift register behaviour : xor the lower 2 bits, shift everything right, and put the xor result as the higher bit (bit 14)
			bool newBit = (m_register & 1) ^ ((m_register >> 1) & 1);
			m_register = ((m_register >> 1) | (newBit << 14));
			if (registerWidth)  // If NR43.3 is set, the xor result is also written in bit 6, so only the 7 lower bits are actually significant
				m_register = (newBit << 6) | (m_register & 0b111111110111111);
			m_baseTimerCounter = 0;
		}
	}

	// Called on every frame that clocks length
	void AudioNoiseMapping::onLengthFrame() {
		if (enableLength) {
			length -= 1;
			if (length == 0)
				disable();
		}
	}

	// Called on every frame that clocks envelope
	void AudioNoiseMapping::onEnvelopeFrame() {
		if (envelopePeriod != 0) {  // Zero period = volume is fixed, no envelope operation
			m_envelopeFrameCounter = (m_envelopeFrameCounter + 1) % envelopePeriod;
			if (m_envelopeFrameCounter == 0){  // Every `envelopePeriod` envelope frames
				if (envelopeDirection && m_envelopeVolume < 15)  // Increasing and not already at max
					m_envelopeVolume += 1;
				else if (!envelopeDirection && m_envelopeVolume > 0)  // Decreasing and not already at zero
					m_envelopeVolume -= 1;
			}
		}
	}

	// Output a sample : digital output is the lower bit of the LFSR, inverted
	float AudioNoiseMapping::buildSample() {
		return ((m_register & 1) ? -1.0f : 1.0f) * m_envelopeVolume / 15;
	}

	// Restart the channel
	void AudioNoiseMapping::reset() {
		// Only start if DAC is enabled (= if the higher 5 bits of NR42 are not all zero)
		if (initialEnvelopeVolume != 0 || envelopeDirection != 0)
			start();

		// If the length counter is set to zero, it is reloaded with maximum
		if (length == 0) {
			if (m_frameSequencer % 2 == 0 && enableLength)
				length = 63;  // If length is enabled and the next frame does not clock length, maximum - 1
			else
				length = 64;
		}
		m_register = 0x7FFF;  // The LFSR starts all set at every restart
		m_envelopeVolume = initialEnvelopeVolume;
	}

	// On APU power off, set everything to zero
	void AudioNoiseMapping::onPowerOff() {
		set(OFFSET_LENGTH, 0);
		set(OFFSET_ENVELOPE, 0);
		set(OFFSET_COUNTER, 0);
		set(OFFSET_CONTROL, 0);
	}

	// On power on, reset the timers
	void AudioNoiseMapping::onPowerOn() {
		m_envelopeFrameCounter = 0;
		m_baseTimerCounter = 0;
	}
}
