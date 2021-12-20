#include "memory/mapping/AudioToneMapping.hpp"

#define OFFSET_START IO_CH2_PATTERN
#define OFFSET_PATTERN  IO_CH2_PATTERN - OFFSET_START
#define OFFSET_ENVELOPE IO_CH2_ENVELOPE - OFFSET_START
#define OFFSET_FREQLOW  IO_CH2_FREQLOW - OFFSET_START
#define OFFSET_CONTROL  IO_CH2_CONTROL - OFFSET_START

namespace toygb {
	AudioToneMapping::AudioToneMapping(int channel, AudioControlMapping* control, HardwareConfig& hardware) : AudioChannelMapping(channel, control, hardware) {
		wavePatternDuty = 0;
		length = 0x3F;

		initialEnvelopeVolume = 0;
		envelopeDirection = false;
		envelopeSweep = 0;

		frequency = 0x07FF;
		stopSelect = false;

		m_dutyPointer = 0;
		m_baseTimerCounter = 0;
		m_envelopeFrameCounter = 0;
	}

	uint8_t AudioToneMapping::get(uint16_t address) {
		switch (address) {
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

	void AudioToneMapping::set(uint16_t address, uint8_t value){
		if (powered || (m_hardware.isDMGConsole() && address == OFFSET_PATTERN)){
			switch (address) {
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

	void AudioToneMapping::onUpdate(){
		m_baseTimerCounter += 1;
		if (m_baseTimerCounter >= 2*(2048 - frequency)){
			m_dutyPointer = (m_dutyPointer + 1) % 8;
			m_baseTimerCounter = 0;
		}
	}

	void AudioToneMapping::onLengthFrame(){
		if (stopSelect){
			length -= 1;
			if (length == 0)
				disable();
		}
	}

	void AudioToneMapping::onEnvelopeFrame(){
		if (envelopeSweep != 0){
			m_envelopeFrameCounter = (m_envelopeFrameCounter + 1) % envelopeSweep;
			if (m_envelopeFrameCounter == 0){
				if (envelopeDirection && m_envelopeVolume < 15)
					m_envelopeVolume += 1;
				else if (m_envelopeVolume > 0)
					m_envelopeVolume -= 1;
			}
		}
	}

	float AudioToneMapping::buildSample(){
		bool patternValue = (TONE_WAVEPATTERNS[wavePatternDuty] >> (7 - m_dutyPointer)) & 1;
		return (patternValue ? 1.0f : -1.0f) * m_envelopeVolume / 15;
	}

	void AudioToneMapping::reset(){
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
	}

	void AudioToneMapping::onPowerOff(){
		set(OFFSET_PATTERN, 0);
		set(OFFSET_ENVELOPE, 0);
		set(OFFSET_FREQLOW, 0);
		set(OFFSET_CONTROL, 0);
	}

	void AudioToneMapping::onPowerOn(){
		m_baseTimerCounter = 0;
		m_envelopeFrameCounter = 0;
		m_dutyPointer = 0;
	}
}
