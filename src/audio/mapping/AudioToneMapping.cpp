#include "audio/mapping/AudioToneMapping.hpp"

#define OFFSET_START IO_CH2_PATTERN
#define OFFSET_PATTERN  IO_CH2_PATTERN - OFFSET_START
#define OFFSET_ENVELOPE IO_CH2_ENVELOPE - OFFSET_START
#define OFFSET_FREQLOW  IO_CH2_FREQLOW - OFFSET_START
#define OFFSET_CONTROL  IO_CH2_CONTROL - OFFSET_START

/** Tone channel control IO registers mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF16 |       0000 | NR21 | BBWWWWWW | Pattern and length control WWLLLLLL
           |            |      |          | - W (bit 6-7) : Wave pattern duty (0-3)
           |            |      |          | - L (bit 0-5) : Set the length counter (0-64)
      FF17 |       0001 | NR22 | BBBBBBBB | Volume envelope control : VVVVDPPP
           |            |      |          | - V (bit 4-7) : Initial volume envelope (0-15)
           |            |      |          | - D (bit 3) : Envelope direction (0 = decrease, 1 = increase)
           |            |      |          | - P (bit 0-2) : Envelope update period (0-7)
           |            |      |          | The digital-to-analog circuit is controlled by bits 3-7 of NR22, and disables the channel if they are all zero
      FF18 |       0002 | NR23 | WWWWWWWW | Lower 8 bits of the frequency control
      FF19 |       0003 | NR24 | WB---WWW | Channel enable control : SL---FFF
           |            |      |          | - S (bit 7) : Trigger the channel operation (Set to 1 to start)
           |            |      |          | - L (bit 6) : Enable the length counter (0 = disabled, 1 = counting, stops the channel when it reaches zero)
           |            |      |          | - F (bits 0-2) : Higher 3 bits of the frequency control */


namespace toygb {
	// Wave pattern duties, NR21.6-7 is the index in this array
	// The channel digital output cycles through the bits in those sequences, from left to right
	const uint8_t TONE_WAVEPATTERNS[4] = {0b00000001, 0b10000001, 0b10000111, 0b01111110};

	// Initialize the channel
	// Most of the initial values are just to have the right values at boot, they will get reset as soon as the channel is used
	AudioToneMapping::AudioToneMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, HardwareConfig& hardware) : AudioChannelMapping(channel, control, debug, hardware) {
		wavePatternDuty = 0;
		length = 0x3F;

		initialEnvelopeVolume = 0;
		envelopeDirection = false;
		envelopePeriod = 0;

		frequency = 0x07FF;
		enableLength = false;

		m_dutyPointer = 0;
		m_baseTimerCounter = 0;
		m_envelopeFrameCounter = 0;
	}

	// Get the value at the given relative address
	uint8_t AudioToneMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_PATTERN:  // NR41
				return (wavePatternDuty << 6) | 0x3F;
			case OFFSET_ENVELOPE:  // NR42
				return (
					(initialEnvelopeVolume << 4) |
					(envelopeDirection << 3) |
					envelopePeriod);
			case OFFSET_FREQLOW:  // NR43
				return 0xFF;
			case OFFSET_CONTROL:  // NR44
				return (enableLength << 6) | 0xBF;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	// Set the value at the given relative address
	void AudioToneMapping::set(uint16_t address, uint8_t value) {
		// Ignore writes when the APU is powered off
		// On DMG hardware, length registers are still fully writable even with the APU powered off
		if (powered || (m_hardware.isDMGConsole() && address == OFFSET_PATTERN)) {
			switch (address) {
				case OFFSET_PATTERN:  // NR41
					if (powered)
						wavePatternDuty = (value >> 6) & 3;
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
				case OFFSET_FREQLOW:  // NR43
					frequency = (frequency & 0x0700) | value;  // Set only the lower 8 of the 11 bits
					break;
				case OFFSET_CONTROL: {  // NR44
					bool wasEnabled = enableLength;
					enableLength = (value >> 6) & 1;

					// Enabling length counter in first half of the length period (next frame does not clock length) clocks length once
					if (!wasEnabled && enableLength && m_frameSequencer % 2 == 0 && length > 0)
						onLengthFrame();

					frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);  // Set only the higher 3 of the 11 bits
					if ((value >> 7) & 1)
						reset();
					break;
				}
			}
		}
	}

	// Called at every APU cycle (= 2 clocks)
	void AudioToneMapping::onUpdate() {
		m_baseTimerCounter += 1;
		// The update period in APU cycles 2*(2048 - `frequency`)
		if (m_baseTimerCounter >= 2*(2048 - frequency)) {
			// Point to the next value of the selected pattern duty
			m_dutyPointer = (m_dutyPointer + 1) % 8;
			m_baseTimerCounter = 0;
		}
	}

	// Called at every frame that clocks length
	void AudioToneMapping::onLengthFrame() {
		if (enableLength) {
			length -= 1;
			if (length == 0)
				disable();
		}
	}
	
	// Called at every envelope frame
	void AudioToneMapping::onEnvelopeFrame() {
		if (envelopePeriod != 0) {  // Envelope does not update if the period is zero
			m_envelopeFrameCounter = (m_envelopeFrameCounter + 1) % envelopePeriod;
			if (m_envelopeFrameCounter == 0) {  // Every `envelopePeriod` envelope frames
				if (envelopeDirection && m_envelopeVolume < 15)  // Increasing and not already at max
					m_envelopeVolume += 1;
				else if (!envelopeDirection && m_envelopeVolume > 0)  // Decreasing and not already zero
					m_envelopeVolume -= 1;
			}
		}
	}

	// Output a sample based on the current channel state
	float AudioToneMapping::buildSample() {
		// Get the current digital sample value on the wave duty
		bool patternValue = (TONE_WAVEPATTERNS[wavePatternDuty] >> (7 - m_dutyPointer)) & 1;
		return (patternValue ? 1.0f : -1.0f) * m_envelopeVolume / 15;
	}

	// Restart the channel
	void AudioToneMapping::reset() {
		// Only start if DAC is enabled (higher 5 bits of NR22)
		if (initialEnvelopeVolume != 0 || envelopeDirection != 0)
			start();

		// If the length counter is set to zero, it is reloaded with maximum
		if (length == 0) {
			if (m_frameSequencer % 2 == 0 && stopSelect)
				length = 63;  // If length is enabled and the next frame does not clock length, maximum - 1
			else
				length = 64;
		}

		m_envelopeVolume = initialEnvelopeVolume;
	}

	// Called on APU power off : set everything to zero
	void AudioToneMapping::onPowerOff() {
		set(OFFSET_PATTERN, 0);
		set(OFFSET_ENVELOPE, 0);
		set(OFFSET_FREQLOW, 0);
		set(OFFSET_CONTROL, 0);
	}

	// Called on APU power on : reset the pointers and duty pointer
	void AudioToneMapping::onPowerOn() {
		m_baseTimerCounter = 0;
		m_envelopeFrameCounter = 0;
		m_dutyPointer = 0;
	}
}
