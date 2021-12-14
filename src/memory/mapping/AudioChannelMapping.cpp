#include "memory/mapping/AudioChannelMapping.hpp"


namespace toygb {
	AudioChannelMapping::AudioChannelMapping(int channel, AudioControlMapping* control, OperationMode mode){
		m_channel = channel;
		m_control = control;
		m_mode = mode;

		m_started = false;
		powered = true;
		m_backBuffer = new float[OUTPUT_BUFFER_SAMPLES];
		m_outputBuffer = new float[OUTPUT_BUFFER_SAMPLES];
		m_outputBufferIndex = 0;
		m_bufferAvailable = false;
	}

	float* AudioChannelMapping::getBuffer(){
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

	void AudioChannelMapping::powerOff(){
		disable();
		onPowerOff();
		powered = false;
	}

	void AudioChannelMapping::powerOn(){
		onPowerOn();
		powered = true;
	}

	void AudioChannelMapping::onPowerOff(){

	}

	void AudioChannelMapping::onPowerOn(){

	}

	void AudioChannelMapping::outputSample(){
		m_backBuffer[m_outputBufferIndex] = (m_started ? buildSample() : 0);

		m_outputBufferIndex = (m_outputBufferIndex + 1) % OUTPUT_BUFFER_SAMPLES;
		if (m_outputBufferIndex == 0){
			float* tmp = m_backBuffer;
			m_backBuffer = m_outputBuffer;
			m_outputBuffer = tmp;
			m_bufferAvailable = true;
		}
	}
}
