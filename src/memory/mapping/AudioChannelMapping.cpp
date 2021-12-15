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

		m_frameSequencer = 0;
		m_frameSequencerTimer = 0;
		m_outputTimerCounter = 0;
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
		m_frameSequencer = 0;
		m_frameSequencerTimer = 0;
		m_outputTimerCounter = 0;
	}

	void AudioChannelMapping::update(){
		m_frameSequencerTimer += 1;
		if (m_frameSequencerTimer >= FRAME_SEQUENCER_PERIOD){
			m_frameSequencerTimer = 0;
			m_frameSequencer = (m_frameSequencer + 1) % 8;
			if (m_frameSequencer >= 8)
				m_frameSequencer = 0;
			onFrame(m_frameSequencer);
		}

		m_outputTimerCounter += 1;
		if (m_outputTimerCounter >= OUTPUT_SAMPLE_PERIOD && m_outputBufferIndex < OUTPUT_BUFFER_SAMPLES){
			m_outputTimerCounter = 0;
			outputSample();
		}

		onUpdate();
	}

	void AudioChannelMapping::onFrame(int frame){
		if (frame % 2 == 0)
			onLengthFrame();

		if (frame == 2 || frame == 6)
			onSweepFrame();

		if (frame == 7)
			onEnvelopeFrame();
	}

	void AudioChannelMapping::onLengthFrame(){

	}

	void AudioChannelMapping::onSweepFrame(){

	}

	void AudioChannelMapping::onEnvelopeFrame(){

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
