#include "memory/mapping/AudioNoiseMapping.hpp"

#define OFFSET_START IO_CH4_LENGTH
#define OFFSET_LENGTH   IO_CH4_LENGTH - OFFSET_START
#define OFFSET_ENVELOPE IO_CH4_ENVELOPE - OFFSET_START
#define OFFSET_COUNTER  IO_CH4_COUNTER - OFFSET_START
#define OFFSET_CONTROL  IO_CH4_CONTROL - OFFSET_START

namespace toygb {
	AudioNoiseMapping::AudioNoiseMapping(int channel, AudioControlMapping* control, OperationMode mode) : AudioChannelMapping(channel, control, mode) {
		length = 0x3F;

		initialEnvelopeVolume = 0;
		envelopeDirection = false;
		envelopeSweep = 0;

		frequency = 0;
		counterStep = false;
		dividingRatio = 0;
		stopSelect = false;

		m_lengthTimerCounter = 0;
		m_outputTimerCounter = 0;
		m_envelopeTimerCounter = 0;
		m_baseTimerCounter = 0;
	}

	uint8_t AudioNoiseMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_LENGTH: return 0xFF;
			case OFFSET_ENVELOPE:
				return (
					(initialEnvelopeVolume << 4) |
					(envelopeDirection << 3) |
					envelopeSweep);
			case OFFSET_COUNTER:
				return (
					(frequency << 4) |
					(counterStep << 3) |
					dividingRatio);
			case OFFSET_CONTROL:
				return (stopSelect << 6) | 0xBF;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	void AudioNoiseMapping::set(uint16_t address, uint8_t value){
		if (powered | (m_mode == OperationMode::DMG && address == OFFSET_LENGTH)){
			switch (address) {
				case OFFSET_LENGTH: length = 64 - (value & 0x3F); break;
				case OFFSET_ENVELOPE:
					initialEnvelopeVolume = (value >> 4) & 0x0F;
					envelopeDirection = (value >> 3) & 1;
					envelopeSweep = value & 7;
					break;
				case OFFSET_COUNTER:
					frequency = (value >> 4) & 0x0F;
					counterStep = (value >> 3) & 1;
					dividingRatio = value & 7;
					break;
				case OFFSET_CONTROL:
					stopSelect = (value >> 6) & 1;
					if ((value >> 7) & 1)
						reset();
					break;
			}
		}
	}

	const int NOISE_DIVISORS[] = {8, 16, 32, 48, 64, 80, 96, 112};

	void AudioNoiseMapping::update() {
		m_lengthTimerCounter += 1;
		if (m_lengthTimerCounter >= LENGTH_TIMER_PERIOD){
			m_lengthTimerCounter = 0;
			length -= 1;
			if (stopSelect && length == 0)
				disable();
		}

		m_baseTimerCounter += 1;
		if (m_baseTimerCounter >= NOISE_DIVISORS[dividingRatio] << frequency){
			bool newBit = (m_register & 1) ^ ((m_register >> 1) & 1);
			m_register = ((m_register >> 1) | (newBit << 14));
			if (counterStep)
				m_register = (newBit << 6) | (m_register & 0b111111110111111);
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

	float AudioNoiseMapping::buildSample() {
		return ((m_register & 1) ? -1.0f : 1.0f) * m_envelopeVolume / 15;
	}

	void AudioNoiseMapping::reset() {
		start();
		m_register = 0x7FFF;
		m_baseTimerCounter = 0;
		m_outputTimerCounter = 0;
		m_envelopeVolume = initialEnvelopeVolume;
	}

	void AudioNoiseMapping::onPowerOff(){
		set(OFFSET_LENGTH, 0);
		set(OFFSET_ENVELOPE, 0);
		set(OFFSET_COUNTER, 0);
		set(OFFSET_CONTROL, 0);
	}

	void AudioNoiseMapping::onPowerOn(){
		m_envelopeTimerCounter = 0;
		m_lengthTimerCounter = 0;
	}
}
