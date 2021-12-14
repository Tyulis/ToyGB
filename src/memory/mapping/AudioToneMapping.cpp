#include "memory/mapping/AudioToneMapping.hpp"

#define OFFSET_START IO_CH2_PATTERN
#define OFFSET_PATTERN  IO_CH2_PATTERN - OFFSET_START
#define OFFSET_ENVELOPE IO_CH2_ENVELOPE - OFFSET_START
#define OFFSET_FREQLOW  IO_CH2_FREQLOW - OFFSET_START
#define OFFSET_CONTROL  IO_CH2_CONTROL - OFFSET_START

namespace toygb {
	AudioToneMapping::AudioToneMapping(int channel, AudioControlMapping* control, OperationMode mode) : AudioChannelMapping(channel, control, mode) {
		wavePatternDuty = 0;
		length = 0x3F;

		initialEnvelopeVolume = 0;
		envelopeDirection = false;
		envelopeSweep = 0;

		frequency = 0x07FF;
		stopSelect = false;

		m_dutyPointer = 0;
		m_baseTimerCounter = 0;
		m_lengthTimerCounter = 0;
		m_outputTimerCounter = 0;
		m_envelopeTimerCounter = 0;
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
		if (powered | (m_mode == OperationMode::DMG && address == OFFSET_PATTERN)){
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
				case OFFSET_CONTROL:
					stopSelect = (value >> 6) & 1;
					frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);
					if ((value >> 7) & 1)
						reset();
					break;
			}
		}
	}

	const uint8_t TONE_WAVEPATTERNS[4] = {0b00000001, 0b10000001, 0b10000111, 0b01111110};

	void AudioToneMapping::update(){
		if (stopSelect){
			m_lengthTimerCounter += 1;
			if (m_lengthTimerCounter >= LENGTH_TIMER_PERIOD){
				m_lengthTimerCounter = 0;
				length -= 1;
				if (length == 0){
					length = 64;
					disable();
				}
			}
		}

		m_baseTimerCounter += 1;
		if (m_baseTimerCounter >= 4*(2048 - frequency)){
			m_dutyPointer = (m_dutyPointer + 1) % 8;
			m_baseTimerCounter = 0;
		}

		if (envelopeSweep != 0){
			m_envelopeTimerCounter += 1;
			if (m_envelopeTimerCounter >= envelopeSweep*ENVELOPE_TIMER_PERIOD){
				m_envelopeTimerCounter = 0;

				if (envelopeDirection && m_envelopeVolume < 15)
					m_envelopeVolume += 1;
				else if (m_envelopeVolume > 0)
					m_envelopeVolume -= 1;
			}
		}

		m_outputTimerCounter += 1;
		if (m_outputTimerCounter >= OUTPUT_SAMPLE_PERIOD && m_outputBufferIndex < OUTPUT_BUFFER_SAMPLES){
			m_outputTimerCounter = 0;
			outputSample();
		}
	}

	float AudioToneMapping::buildSample(){
		bool patternValue = (TONE_WAVEPATTERNS[wavePatternDuty] >> (7 - m_dutyPointer)) & 1;
		return (patternValue ? 1.0f : -1.0f) * m_envelopeVolume / 15;
	}

	void AudioToneMapping::reset(){
		if (initialEnvelopeVolume != 0 || envelopeDirection != 0){
			start();
			if (length == 0)
				length = 64;
			m_outputTimerCounter = 0;
			m_baseTimerCounter = 0;
			m_envelopeVolume = initialEnvelopeVolume;
		}
	}

	void AudioToneMapping::onPowerOff(){
		set(OFFSET_PATTERN, 0);
		set(OFFSET_ENVELOPE, 0);
		set(OFFSET_FREQLOW, 0);
		set(OFFSET_CONTROL, 0);
	}

	void AudioToneMapping::onPowerOn(){
		m_lengthTimerCounter = 0;
		m_envelopeTimerCounter = 0;
		m_dutyPointer = 0;
	}
}
