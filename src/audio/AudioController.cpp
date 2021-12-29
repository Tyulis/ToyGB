#include "audio/AudioController.hpp"

#define CHANNEL_TONE_SWEEP 0
#define CHANNEL_TONE 1
#define CHANNEL_WAVE 2
#define CHANNEL_NOISE 3

#define SAMPLE_LOW (-2000)
#define SAMPLE_HIGH (2000)


namespace toygb {
	// Constructor, only initialize with null values (the actual initialization is done by AudioController::init)
	AudioController::AudioController() {
		for (int i = 0; i < 4; i++)
			m_channels[i] = nullptr;
		m_control = nullptr;
		m_wavePatternMapping = nullptr;
		m_wavePattern = nullptr;
	}

	AudioController::~AudioController() {
		if (m_wavePattern != nullptr) delete[] m_wavePattern;
		if (m_wavePatternMapping != nullptr) delete m_wavePatternMapping;
		if (m_control != nullptr) delete m_control;
		for (int i = 0; i < 4; i++){
			if (m_channels[i]){
				delete m_channels[i];
				m_channels[i] = nullptr;
			}
		}
	}

	// Initialize the component
	void AudioController::init(HardwareConfig* hardware) {
		m_hardware = hardware;
		m_wavePattern = new uint8_t[IO_WAVEPATTERN_SIZE];

		m_wavePatternMapping = new WaveMemoryMapping(m_wavePattern, m_hardware);
		m_control = new AudioControlMapping(m_hardware);
		m_debug = new AudioDebugMapping(m_hardware);
		m_channels[0] = new AudioToneSweepMapping(0, m_control, m_debug, m_hardware);
		m_channels[1] = new AudioToneMapping(1, m_control, m_debug, m_hardware);
		m_channels[2] = new AudioWaveMapping(2, m_control, m_debug, m_wavePatternMapping, m_hardware);
		m_channels[3] = new AudioNoiseMapping(3, m_control, m_debug, m_hardware);
	}

	// Configure the component's memory mappings
	void AudioController::configureMemory(MemoryMap* memory) {
		memory->add(IO_CH1_SWEEP, IO_CH1_CONTROL, m_channels[0]);
		memory->add(IO_CH2_PATTERN, IO_CH2_CONTROL, m_channels[1]);
		memory->add(IO_CH3_ENABLE, IO_CH3_CONTROL, m_channels[2]);
		memory->add(IO_CH4_LENGTH, IO_CH4_CONTROL, m_channels[3]);
		memory->add(IO_AUDIO_LEVELS, IO_AUDIO_ENABLE, m_control);
		memory->add(IO_WAVEPATTERN_START, IO_WAVEPATTERN_END, m_wavePatternMapping);
		memory->add(IO_UNDOCUMENTED_FF72, IO_PCM34, m_debug);
	}

#define clock(num) m_cyclesToSkip = num-1; \
			  co_await std::suspend_always();

	// Main component loop (implemented as a coroutine)
	GBComponent AudioController::run() {
		while (true) {
			for (int index = 0; index < 4; index++) {  // Update each channel
				AudioChannelMapping* channel = m_channels[index];
				if (m_control->audioEnable) {
					if (!channel->powered)  // Enable set but not powered : audio controller just got powered on
						channel->powerOn();
					channel->update();
				} else if (channel->powered) {  // Enable clear but powered : audio controller just got powered off
					channel->powerOff();
				}
			}

			clock(2);
		}
	}

	// Tell whether the emulator can skip running this component for the cycle, to save a context commutation if running it is useless
	bool AudioController::skip() {
		if (m_cyclesToSkip > 0) {  // We are in-between APU cycles (= 2 clocks)
			m_cyclesToSkip -= 1;
			return true;
		}

		// Skip if the APU is disabled and the shutdown is already done
		return !m_control->audioEnable && !m_channels[0]->powered && !m_channels[1]->powered && !m_channels[2]->powered && !m_channels[3]->powered;
	}

	// Get the mixed samples if available
	bool AudioController::getSamples(int16_t* buffer) {
		// Clear the sample buffer first
		for (int i = 0; i < 2*OUTPUT_BUFFER_SAMPLES; i++)
			buffer[i] = 0;

		// The APU is powered off, so just return the buffer filled with zeros
		if (!m_control->audioEnable)
			return true;

		// Get each channel’s buffer
		float* channelBuffers[4];
		for (int channel = 0; channel < 4; channel++) {
			if ((channelBuffers[channel] = m_channels[channel]->getBuffer()) == nullptr)
				return false;  // Channel returned nullptr -> not enough samples generated for the moment
		}

		// Mix samples
		for (int channel = 0; channel < 4; channel++) {
			for (int sample = 0; sample < OUTPUT_BUFFER_SAMPLES; sample++) {
				// Output 2 is left, output 1 is right
				//                 if output is enabled for the channel  : output volume               * sample value     / output level is in range 0-7 -> 8
				float leftValue =  (m_control->output2Channels[channel]) ? (m_control->output2Level+1) * channelBuffers[channel][sample] / 8 : 0;
				float rightValue = (m_control->output1Channels[channel]) ? (m_control->output1Level+1) * channelBuffers[channel][sample] / 8 : 0;

				// Our output buffer has left first
				buffer[2*sample] += int16_t(leftValue * 2400);
				buffer[2*sample+1] += int16_t(rightValue * 2400);
			}
		}
		return true;
	}
}
