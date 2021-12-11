#include "memory/mapping/AudioChannelMapping.hpp"


namespace toygb {
	AudioChannelMapping::AudioChannelMapping(int channel, AudioControlMapping* control){
		m_channel = channel;
		m_control = control;

		m_started = false;
		m_backBuffer = new int16_t[2*OUTPUT_BUFFER_SAMPLES];
		m_outputBuffer = new int16_t[2*OUTPUT_BUFFER_SAMPLES];
		m_outputBufferIndex = 0;
		m_bufferAvailable = false;
	}

	int16_t* AudioChannelMapping::getBuffer(){
		if (m_bufferAvailable){
			m_bufferAvailable = false;
			return m_outputBuffer;
		} else {
			return nullptr;
		}
	}

	void AudioChannelMapping::start(){
		m_started = true;
		m_control->channelEnable[m_channel] = true;
	}

	void AudioChannelMapping::disable(){
		m_control->channelEnable[m_channel] = false;
		m_started = false;
	}

	void AudioChannelMapping::outputSample(){
		m_backBuffer[2*m_outputBufferIndex] = m_backBuffer[2*m_outputBufferIndex+1] = 0;
		if (m_started){
			uint16_t outputValue = buildSample();
			m_backBuffer[2*m_outputBufferIndex] = (m_control->output2Channels[m_channel]) ? (m_control->output2Level+1) * outputValue / 8 : 0;
			m_backBuffer[2*m_outputBufferIndex+1] = (m_control->output1Channels[m_channel]) ? (m_control->output1Level+1) * outputValue / 8 : 0;
		}

		m_outputBufferIndex = (m_outputBufferIndex + 1) % OUTPUT_BUFFER_SAMPLES;
		if (m_outputBufferIndex == 0){
			int16_t* tmp = m_backBuffer;
			m_backBuffer = m_outputBuffer;
			m_outputBuffer = tmp;
			m_bufferAvailable = true;
		}
	}
}
