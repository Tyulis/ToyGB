#include "audio/mapping/AudioChannelMapping.hpp"


namespace toygb {
	// Initialize the base channel
	AudioChannelMapping::AudioChannelMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, HardwareConfig* hardware){
		m_channel = channel;
		m_control = control;
		m_debug = debug;
		m_hardware = hardware;

		m_started = false;  // Start disabled and powered on
		powered = true;

		m_backBuffer = new float[OUTPUT_BUFFER_SAMPLES];
		m_outputBuffer = new float[OUTPUT_BUFFER_SAMPLES];
		m_outputBufferIndex = 0;
		m_bufferAvailable = false;

		// Default values for the frame sequencer, not confirmed
		m_frameSequencer = 7;
		m_frameSequencerTimer = 0;
		m_outputTimerCounter = 0;
	}

	// Get a full sample buffer, or nullptr if not full yed
	float* AudioChannelMapping::getBuffer(){
		if (m_bufferAvailable){
			m_bufferAvailable = false;
			return m_outputBuffer;
		} else {
			return nullptr;
		}
	}

	// Enable channel operation, usually via NRx4.7
	void AudioChannelMapping::start(){
		m_started = true;
		m_control->channelEnable[m_channel] = true;  // Set the associated NR52 bit
	}

	// Disable channel operation
	void AudioChannelMapping::disable(){
		m_control->channelEnable[m_channel] = false;
		m_started = false;
	}

	// Called on APU power off
	void AudioChannelMapping::powerOff(){
		disable();
		onPowerOff();
		powered = false;
	}

	// Called on APU power on
	void AudioChannelMapping::powerOn(){
		onPowerOn();
		powered = true;

		// Initial value for the frame sequencer on power-on, confirmed
		m_frameSequencer = 7;
		m_frameSequencerTimer = 0;
	}

	// Called at every APU cycle
	void AudioChannelMapping::update(){
		// Frame sequencer operation
		m_frameSequencerTimer += 1;
		if (m_frameSequencerTimer >= FRAME_SEQUENCER_PERIOD){  // Passed one 512Hz clock
			m_frameSequencerTimer = 0;
			// Frame cycles in range 0-8
			m_frameSequencer = (m_frameSequencer + 1) % 8;
			if (m_frameSequencer >= 8)
				m_frameSequencer = 0;
			onFrame(m_frameSequencer);
		}

		// Audio output operation
		m_outputTimerCounter += 1;
		if (m_outputTimerCounter >= OUTPUT_SAMPLE_PERIOD){
			m_outputTimerCounter = 0;
			outputSample();
		}

		onUpdate();
	}

	/** Called every time the frame sequencer clocks
	 * Sub-clocks frames are as follow (https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware)
	 * Frame | Length | Envelope | Sweep
	 *     0 |  Clock |          |
	 *     1 |        |          |
	 *     2 |  Clock |          | Clock
	 *     3 |        |          |
	 *     4 |  Clock |          |
	 *     5 |        |          |
	 *     6 |  Clock |          | Clock
	 *     7 |        |    Clock | */
	void AudioChannelMapping::onFrame(int frame){
		if (frame % 2 == 0)  // Length frames are just every 2 frames
			onLengthFrame();

		if (frame == 2 || frame == 6)
			onSweepFrame();

		if (frame == 7)
			onEnvelopeFrame();
	}

	// Default, no-op implementation for those
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

	// Output a sample to the audio output buffer
	void AudioChannelMapping::outputSample(){
		float sample = (m_started ? buildSample() : 0);
		m_backBuffer[m_outputBufferIndex] = sample;
		m_debug->setChannelAmplitude(m_channel, uint8_t(((sample + 1.0f) / 2) * 0x0F));

		// Swap the buffers once the back buffer is full
		m_outputBufferIndex = (m_outputBufferIndex + 1) % OUTPUT_BUFFER_SAMPLES;
		if (m_outputBufferIndex == 0){
			float* tmp = m_backBuffer;
			m_backBuffer = m_outputBuffer;
			m_outputBuffer = tmp;
			m_bufferAvailable = true;
		}
	}
}
