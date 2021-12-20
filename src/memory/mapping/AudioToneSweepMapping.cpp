#include "memory/mapping/AudioToneSweepMapping.hpp"

#define OFFSET_START IO_CH1_SWEEP
#define OFFSET_SWEEP    IO_CH1_SWEEP - OFFSET_START
#define OFFSET_PATTERN  IO_CH1_PATTERN - OFFSET_START
#define OFFSET_ENVELOPE IO_CH1_ENVELOPE - OFFSET_START
#define OFFSET_FREQLOW  IO_CH1_FREQLOW - OFFSET_START
#define OFFSET_CONTROL  IO_CH1_CONTROL - OFFSET_START

namespace toygb {
	AudioToneSweepMapping::AudioToneSweepMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, HardwareConfig& hardware) : AudioChannelMapping(channel, control, debug, hardware) {
		sweepTime = 0;
		sweepDirection = false;
		sweepShift = 0;

		wavePatternDuty = 2;
		length = 0x3F;

		initialEnvelopeVolume = 0x0F;
		envelopeDirection = false;
		envelopeSweep = 3;

		frequency = 0x07FF;
		stopSelect = false;

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
			case OFFSET_SWEEP:
				return (0x80 |
					(sweepTime << 4) |
					(sweepDirection << 3) |
					sweepShift);
			case OFFSET_PATTERN: return (wavePatternDuty << 6) | 0x3F;
			case OFFSET_ENVELOPE:
				return (
					(initialEnvelopeVolume << 4) |
					(envelopeDirection << 3) |
					envelopeSweep);
			case OFFSET_FREQLOW: return 0xFF;
			case OFFSET_CONTROL:
				return (stopSelect << 6) | 0xBF;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	void AudioToneSweepMapping::set(uint16_t address, uint8_t value){
		if (powered || (m_hardware.isDMGConsole() && address == OFFSET_PATTERN)){
			switch (address) {
				case OFFSET_SWEEP:
					sweepTime = (value >> 4) & 7;
					// Disable the channel when clearing the direction bit after sweep has been calculated at least one
					if (!((value >> 3) & 1) && sweepDirection && m_sweepNegateCalculated)
						disable();
					sweepDirection = (value >> 3) & 1;
					sweepShift = value & 7;
					break;
				case OFFSET_PATTERN:
					if (powered)
						wavePatternDuty = (value >> 6) & 3;
					length = 64 - (value & 0x3F);
					break;
				case OFFSET_ENVELOPE:
					if ((value & 0xF8) == 0)
						disable();
					initialEnvelopeVolume = (value >> 4) & 0x0F;
					envelopeDirection = (value >> 3) & 1;
					envelopeSweep = value & 7;
					break;
				case OFFSET_FREQLOW:
					frequency = (frequency & 0x0700) | value;
					break;
				case OFFSET_CONTROL: {
					// If enabling length in first half of the length period
					bool wasEnabled = stopSelect;
					stopSelect = (value >> 6) & 1;
					if (!wasEnabled && stopSelect && m_frameSequencer % 2 == 0 && length > 0)
						onLengthFrame();

					frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);
					if ((value >> 7) & 1)
						reset();
					break;
				}
			}
		}
	}

	const uint8_t TONE_WAVEPATTERNS[4] = {0b00000001, 0b10000001, 0b10000111, 0b01111110};

	void AudioToneSweepMapping::onUpdate(){
		m_baseTimerCounter += 1;
		if (m_baseTimerCounter >= 2*(2048 - m_sweepFrequency)){
			m_dutyPointer = (m_dutyPointer + 1) % 8;
			m_baseTimerCounter = 0;
		}
	}

	void AudioToneSweepMapping::onLengthFrame(){
		if (stopSelect){
			length -= 1;
			if (length == 0)
				disable();
		}
	}

	void AudioToneSweepMapping::onSweepFrame(){
		if (m_started){
			m_sweepFrameCounter -= 1;
			if (m_sweepFrameCounter <= 0){
				m_sweepFrameCounter = (sweepTime == 0 ? 8 : sweepTime);
				if (m_sweepEnabled && sweepTime != 0){
					uint16_t newFrequency = calculateFrequencySweep();
					if (newFrequency < 2048 && sweepShift != 0){
						m_sweepFrequency = newFrequency;
						frequency = newFrequency;
						calculateFrequencySweep();
					}
				}
			}
		}
	}

	void AudioToneSweepMapping::onEnvelopeFrame(){
		if (envelopeSweep != 0){
			m_envelopeFrameCounter -= 1;
			if (m_envelopeFrameCounter <= 0){
				m_envelopeFrameCounter = (envelopeSweep == 0 ? 8 : envelopeSweep);
				if (envelopeDirection && m_envelopeVolume < 15)
					m_envelopeVolume += 1;
				else if (m_envelopeVolume > 0)
					m_envelopeVolume -= 1;
			}
		}
	}

	float AudioToneSweepMapping::buildSample(){
		bool patternValue = (TONE_WAVEPATTERNS[wavePatternDuty] >> (7 - m_dutyPointer)) & 1;
		return (patternValue ? 1.0f : -1.0f) * m_envelopeVolume / 15;
	}

	uint16_t AudioToneSweepMapping::calculateFrequencySweep(){
		uint16_t newFrequency = m_sweepFrequency;
		if (sweepDirection){
			newFrequency -= (m_sweepFrequency >> sweepShift);
			m_sweepNegateCalculated = true;
		} else {
			newFrequency += (m_sweepFrequency >> sweepShift);
		}

		// Overflow check
		if (newFrequency > 2047)
			disable();

		return newFrequency;
	}

	void AudioToneSweepMapping::reset(){
		// Only start if DAC is enabled
		if (initialEnvelopeVolume != 0 || envelopeDirection != 0)
			start();

		if (length == 0){
			if (m_frameSequencer % 2 == 0 && stopSelect)
				length = 63;
			else
				length = 64;
		}

		m_envelopeVolume = initialEnvelopeVolume;
		m_sweepFrequency = frequency;
		m_sweepFrameCounter = (sweepTime == 0 ? 8 : sweepTime);

		m_sweepEnabled = (sweepShift != 0 || sweepTime != 0);
		m_sweepNegateCalculated = false;

		if (sweepShift != 0)
			calculateFrequencySweep();
	}

	void AudioToneSweepMapping::onPowerOff(){
		set(OFFSET_SWEEP, 0);
		set(OFFSET_PATTERN, 0);
		set(OFFSET_ENVELOPE, 0);
		set(OFFSET_FREQLOW, 0);
		set(OFFSET_CONTROL, 0);
	}

	void AudioToneSweepMapping::onPowerOn(){
		m_dutyPointer = 0;
		m_outputTimerCounter = 0;
		m_baseTimerCounter = 0;
		m_envelopeFrameCounter = 0;
		m_sweepFrameCounter = 0;
		m_sweepEnabled = false;
	}
}
