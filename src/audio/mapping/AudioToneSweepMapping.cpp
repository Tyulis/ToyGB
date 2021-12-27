#include "audio/mapping/AudioToneSweepMapping.hpp"

#define OFFSET_START IO_CH1_SWEEP
#define OFFSET_SWEEP    IO_CH1_SWEEP - OFFSET_START
#define OFFSET_PATTERN  IO_CH1_PATTERN - OFFSET_START
#define OFFSET_ENVELOPE IO_CH1_ENVELOPE - OFFSET_START
#define OFFSET_FREQLOW  IO_CH1_FREQLOW - OFFSET_START
#define OFFSET_CONTROL  IO_CH1_CONTROL - OFFSET_START

/** Tone channel with frequency sweep control IO registers mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF10 |       0000 | NR10 | -BBBBBBB | Frequency sweep control :
           |            |      |          |
      FF11 |       0001 | NR21 | BBWWWWWW | Pattern and length control WWLLLLLL
           |            |      |          | - W (bit 6-7) : Wave pattern duty (0-3)
           |            |      |          | - L (bit 0-5) : Set the length counter (0-64)
      FF12 |       0002 | NR22 | BBBBBBBB | Volume envelope control : VVVVDPPP
           |            |      |          | - V (bit 4-7) : Initial volume envelope (0-15)
           |            |      |          | - D (bit 3) : Envelope direction (0 = decrease, 1 = increase)
           |            |      |          | - P (bit 0-2) : Envelope update period (0-7)
           |            |      |          | The digital-to-analog circuit is controlled by bits 3-7 of NR22, and disables the channel if they are all zero
      FF13 |       0003 | NR23 | WWWWWWWW | Lower 8 bits of the frequency control
      FF14 |       0004 | NR24 | WB---WWW | Channel enable control : SL---FFF
           |            |      |          | - S (bit 7) : Trigger the channel operation (Set to 1 to start)
           |            |      |          | - L (bit 6) : Enable the length counter (0 = disabled, 1 = counting, stops the channel when it reaches zero)
           |            |      |          | - F (bits 0-2) : Higher 3 bits of the frequency control */

namespace toygb {
	// Wave pattern duties, NR21.6-7 is the index in this array
	// The channel digital output cycles through the bits in those sequences, from left to right
	const uint8_t TONE_WAVEPATTERNS[4] = {0b00000001, 0b10000001, 0b10000111, 0b01111110};

	// Initialize the channel
	// Most of the initial values are just to have the right values at boot, they will get reset as soon as the channel is used
	AudioToneSweepMapping::AudioToneSweepMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, HardwareConfig& hardware) : AudioChannelMapping(channel, control, debug, hardware) {
		sweepPeriod = 0;
		sweepDirection = false;
		sweepShift = 0;

		wavePatternDuty = 2;
		length = 0x3F;

		initialEnvelopeVolume = 0x0F;
		envelopeDirection = false;
		envelopePeriod = 3;

		frequency = 0x07FF;
		enableLength = false;

		m_dutyPointer = 0;
		m_baseTimerCounter = 0;
		m_outputTimerCounter = 0;
		m_sweepFrequency = 0;
		m_envelopeFrameCounter = 0;
		m_sweepFrameCounter = 0;
		m_sweepEnabled = false;
		m_sweepNegateCalculated = false;
	}

	uint8_t AudioToneSweepMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_SWEEP:  // NR10
				return (0x80 |
					(sweepPeriod << 4) |
					(sweepDirection << 3) |
					sweepShift);
			case OFFSET_PATTERN:  // NR11
				return (wavePatternDuty << 6) | 0x3F;
			case OFFSET_ENVELOPE:  // NR12
				return (
					(initialEnvelopeVolume << 4) |
					(envelopeDirection << 3) |
					envelopePeriod);
			case OFFSET_FREQLOW:  // NR13
				return 0xFF;
			case OFFSET_CONTROL:  // NR14
				return (enableLength << 6) | 0xBF;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	void AudioToneSweepMapping::set(uint16_t address, uint8_t value){
		// Ignore writes when the APU is powered off
		// On DMG hardware, length registers are still fully writable even with the APU powered off
		if (powered || (m_hardware.isDMGConsole() && address == OFFSET_PATTERN)) {
			switch (address) {
				case OFFSET_SWEEP:  // NR10
					sweepPeriod = (value >> 4) & 7;
					// Disable the channel when clearing the direction bit after sweep has been calculated at least once in negate mode
					if (!((value >> 3) & 1) && sweepDirection && m_sweepNegateCalculated)
						disable();
					sweepDirection = (value >> 3) & 1;
					sweepShift = value & 7;
					break;
				case OFFSET_PATTERN:  // NR11
					if (powered)
						wavePatternDuty = (value >> 6) & 3;
					length = 64 - (value & 0x3F);
					break;
				case OFFSET_ENVELOPE:  // NR12
					// The higher 5 bits control the DAC : if they are all 0, the channel is disabled
					if ((value & 0xF8) == 0)
						disable();
					initialEnvelopeVolume = (value >> 4) & 0x0F;
					envelopeDirection = (value >> 3) & 1;
					envelopePeriod = value & 7;
					break;
				case OFFSET_FREQLOW:  // NR13
					frequency = (frequency & 0x0700) | value;  // Set the lower 8 of the 11 bits
					break;
				case OFFSET_CONTROL: {  // NR14
					bool wasEnabled = enableLength;
					enableLength = (value >> 6) & 1;

					// Enabling length counter in first half of the length period (next frame does not clock length) clocks length once
					if (!wasEnabled && enableLength && m_frameSequencer % 2 == 0 && length > 0)
						onLengthFrame();

					frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);  // Set the higher 3 of the 11 bits
					if ((value >> 7) & 1)  // Restart bit
						reset();
					break;
				}
			}
		}
	}

	// Called at every APU cycle (= 2 clocks)
	// Period calculation is the same as channel 2, but using the calculated frequency with sweep
	void AudioToneSweepMapping::onUpdate() {
		m_baseTimerCounter += 1;
		if (m_baseTimerCounter >= 2*(2048 - m_sweepFrequency)) {
			m_dutyPointer = (m_dutyPointer + 1) % 8;
			m_baseTimerCounter = 0;
		}
	}

	// Called at every frame that clocks length
	void AudioToneSweepMapping::onLengthFrame() {
		if (enableLength) {
			length -= 1;
			if (length == 0)
				disable();
		}
	}

	// Called at every sweep frame
	void AudioToneSweepMapping::onSweepFrame() {
		if (m_started) {
			m_sweepFrameCounter -= 1;
			if (m_sweepFrameCounter <= 0) {  // Every `sweepPeriod` sweep frames
				m_sweepFrameCounter = (sweepPeriod == 0 ? 8 : sweepPeriod);  // A period of 0 makes the frequency sweep tick every 8 sweep frames
				if (m_sweepEnabled && sweepPeriod != 0){
					// Perform the first frequency recalculation
					uint16_t newFrequency = calculateFrequencySweep();
					if (newFrequency < 2048 && sweepShift != 0) {  // No overflow
						m_sweepFrequency = newFrequency;  // Update the frequency
						frequency = newFrequency;
						calculateFrequencySweep();  // Perform a second calculation with overflow check, but the frequency value is not used
					}
				}
			}
		}
	}

	// Called on every envelope frame
	void AudioToneSweepMapping::onEnvelopeFrame() {
		if (envelopePeriod != 0) {  // Envelope does not update if the period is zero
			m_envelopeFrameCounter -= 1;
			if (m_envelopeFrameCounter <= 0) {  // Every `envelopePeriod` envelope frames
				m_envelopeFrameCounter = (envelopePeriod == 0 ? 8 : envelopePeriod);  // A period of 0 makes it tick every 8 envelope frames
				if (envelopeDirection && m_envelopeVolume < 15)  // Increasing and not already at max
					m_envelopeVolume += 1;
				else if (!envelopeDirection && m_envelopeVolume > 0)  // Decreasing and not already at zero
					m_envelopeVolume -= 1;
			}
		}
	}

	// Output a sample based on the current state of the channel
	float AudioToneSweepMapping::buildSample() {
		// Get the current digital sample value on the wave duty
		bool patternValue = (TONE_WAVEPATTERNS[wavePatternDuty] >> (7 - m_dutyPointer)) & 1;
		return (patternValue ? 1.0f : -1.0f) * m_envelopeVolume / 15;
	}

	// Perform a frequency sweep calculation and overflow check, and return the calculated frequency
	uint16_t AudioToneSweepMapping::calculateFrequencySweep() {
		uint16_t newFrequency = m_sweepFrequency;
		if (sweepDirection) {  // Decreasing
			newFrequency -= (m_sweepFrequency >> sweepShift);
			m_sweepNegateCalculated = true;  // Need to mark it to disable the channel correctly if the sweep direction is inverted
		} else {  // Increasin
			newFrequency += (m_sweepFrequency >> sweepShift);
		}

		// Overflow check : Disable the channel if the frequence overflows its 11 bits
		if (newFrequency > 2047)
			disable();

		return newFrequency;
	}

	// Resqart the channel operation
	void AudioToneSweepMapping::reset() {
		// Only start if DAC is enabled
		if (initialEnvelopeVolume != 0 || envelopeDirection != 0)
			start();

		// If the length counter is set to zero, it is reloaded with maximum
		if (length == 0){
			// If length is enabled and the next frame does not clock length, maximum - 1
			if (m_frameSequencer % 2 == 0 && enableLength)
				length = 63;
			else
				length = 64;
		}

		m_envelopeVolume = initialEnvelopeVolume;
		m_sweepFrequency = frequency;
		m_sweepFrameCounter = (sweepPeriod == 0 ? 8 : sweepPeriod);  // Start at maximum, a period of 0 makes it tick every 8 sweep frames

		m_sweepEnabled = (sweepShift != 0 || sweepPeriod != 0);  // The internal sweep enable flag is set if either is non-zero
		m_sweepNegateCalculated = false;

		if (sweepShift != 0)  // If the sweep shift is non-zero, a calculation and overflow check is done immediately and the value is not used
			calculateFrequencySweep();
	}

	// Called when the APU is powered off, set everything to zero
	void AudioToneSweepMapping::onPowerOff() {
		set(OFFSET_SWEEP, 0);
		set(OFFSET_PATTERN, 0);
		set(OFFSET_ENVELOPE, 0);
		set(OFFSET_FREQLOW, 0);
		set(OFFSET_CONTROL, 0);
	}

	// Called whe' the APU is powered on, reset the timers, sweep and wave duty position
	void AudioToneSweepMapping::onPowerOn(){
		m_dutyPointer = 0;
		m_outputTimerCounter = 0;
		m_baseTimerCounter = 0;
		m_envelopeFrameCounter = 0;
		m_sweepFrameCounter = 0;
		m_sweepEnabled = false;
	}
}
