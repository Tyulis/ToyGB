#include "memory/mapping/AudioToneMapping.hpp"

#define OFFSET_START IO_CH2_PATTERN
#define OFFSET_PATTERN  IO_CH2_PATTERN - OFFSET_START
#define OFFSET_ENVELOPE IO_CH2_ENVELOPE - OFFSET_START
#define OFFSET_FREQLOW  IO_CH2_FREQLOW - OFFSET_START
#define OFFSET_CONTROL  IO_CH2_CONTROL - OFFSET_START

namespace toygb {
	AudioToneMapping::AudioToneMapping(int channel, AudioControlMapping* control) {
		m_channel = channel;
		m_control = control;

		wavePatternDuty = 0;
		length = 0x3F;

		initialEnvelopeVolume = 0;
		envelopeDirection = false;
		envelopeSweep = 0;

		frequency = 0x07FF;
		stopSelect = false;

		started = false;

		m_baseTimer = 0;
		m_baseTimerCounter = 0;
		m_lengthTimerCounter = 0;
		m_outputTimerCounter = 0;
		m_outputBuffer = new int16_t[2*OUTPUT_BUFFER_SAMPLES];
		m_backBuffer = new int16_t[2*OUTPUT_BUFFER_SAMPLES];
		m_outputBufferIndex = 0;
		m_bufferAvailable = false;
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
		switch (address) {
			case OFFSET_PATTERN:
				wavePatternDuty = (value >> 6) & 3;
				length = value & 0x3F;
				break;
			case OFFSET_ENVELOPE:
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

	const uint8_t TONE_WAVEPATTERNS[4] = {0b00000001, 0b10000001, 0b10000111, 0b01111110};

	void AudioToneMapping::update(){
		m_lengthTimerCounter += 1;
		if (m_lengthTimerCounter >= LENGTH_TIMER_PERIOD){
			m_lengthTimerCounter = 0;
			length -= 1;
			if (stopSelect && length == 0)
				disable();
		}

		m_baseTimerCounter += 1;
		if (m_baseTimerCounter >= 4*(2048 - frequency)){
			m_baseTimer += 1;
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

	void AudioToneMapping::outputSample(){
		if (started){
			int periodSample = 7 - (m_baseTimer % 8);
			bool patternValue = (TONE_WAVEPATTERNS[wavePatternDuty] >> periodSample) & 1;
			int16_t outputValue = (patternValue ? 2000 : -2000) * m_envelopeVolume / 15;

			m_backBuffer[2*m_outputBufferIndex] = (m_control->output2Channels[m_channel]) ? (m_control->output2Level+1) * outputValue / 8 : 0;
			m_backBuffer[2*m_outputBufferIndex+1] = (m_control->output1Channels[m_channel]) ? (m_control->output1Level+1) * outputValue / 8 : 0;
		} else {
			m_backBuffer[2*m_outputBufferIndex] = m_backBuffer[2*m_outputBufferIndex+1] = 0;
		}

		m_outputBufferIndex = (m_outputBufferIndex + 1) % OUTPUT_BUFFER_SAMPLES;
		if (m_outputBufferIndex == 0){
			int16_t* tmp = m_backBuffer;
			m_backBuffer = m_outputBuffer;
			m_outputBuffer = tmp;
			m_bufferAvailable = true;
		}
	}

	int16_t* AudioToneMapping::getBuffer() {
		if (m_bufferAvailable){
			m_bufferAvailable = false;
			return m_outputBuffer;
		} else {
			return nullptr;
		}
	}

	void AudioToneMapping::reset(){
		started = true;
		m_control->channelEnable[m_channel] = true;
		m_outputTimerCounter = 0;
		m_baseTimerCounter = 0;
		m_lengthTimerCounter = 0;
		m_envelopeTimerCounter = 0;
		m_baseTimer = 0;
		m_envelopeVolume = initialEnvelopeVolume;
		//std::cout << "Tone : pattern=" << oh8(wavePatternDuty) << ", length=" << oh8(length) << ", volume=" << oh8(initialEnvelopeVolume) << ", direction=" << envelopeDirection << ", sweep=" << oh8(envelopeSweep)
		//          << ", frequency=" << oh16(frequency) << ", stop=" << stopSelect << ", started=" << started << std::endl;
	}

	void AudioToneMapping::disable(){
		m_control->channelEnable[m_channel] = false;
		started = false;
	}
}
