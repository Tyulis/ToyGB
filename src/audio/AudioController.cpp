#include "audio/AudioController.hpp"

#define CHANNEL_TONE_SWEEP 0
#define CHANNEL_TONE 1
#define CHANNEL_WAVE 2
#define CHANNEL_NOISE 3


#define SAMPLE_LOW (-2000)
#define SAMPLE_HIGH (2000)

namespace toygb {
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

	void AudioController::init(OperationMode mode){
		m_mode = mode;
		m_wavePattern = new uint8_t[IO_WAVEPATTERN_SIZE];

		m_wavePatternMapping = new WaveMemoryMapping(m_wavePattern, m_mode);
		m_control = new AudioControlMapping();
		m_channels[0] = new AudioToneSweepMapping(0, m_control, m_mode);
		m_channels[1] = new AudioToneMapping(1, m_control, m_mode);
		m_channels[2] = new AudioWaveMapping(2, m_control, m_wavePatternMapping, m_mode);
		m_channels[3] = new AudioNoiseMapping(3, m_control, m_mode);
	}

	void AudioController::configureMemory(MemoryMap* memory) {
		memory->add(IO_CH1_SWEEP, IO_CH1_CONTROL, m_channels[0]);
		memory->add(IO_CH2_PATTERN, IO_CH2_CONTROL, m_channels[1]);
		memory->add(IO_CH3_ENABLE, IO_CH3_CONTROL, m_channels[2]);
		memory->add(IO_CH4_LENGTH, IO_CH4_CONTROL, m_channels[3]);
		memory->add(IO_AUDIO_LEVELS, IO_AUDIO_ENABLE, m_control);
		memory->add(IO_WAVEPATTERN_START, IO_WAVEPATTERN_END, m_wavePatternMapping);
	}

#define dot() co_await std::suspend_always()

	GBComponent AudioController::run(){
		while (true){
			for (int index = 0; index < 4; index++){
				AudioChannelMapping* channel = m_channels[index];
				if (m_control->audioEnable){
					if (!channel->powered)
						channel->powerOn();
					channel->update();
				} else if (channel->powered){
					channel->powerOff();
				}
			}
			dot();
			dot();
		}
	}

	bool AudioController::getSamples(int16_t* buffer){
		for (int i = 0; i < 2*OUTPUT_BUFFER_SAMPLES; i++)
			buffer[i] = 0;

		float* channelBuffers[4];
		for (int channel = 0; channel < 4; channel++){
			if ((channelBuffers[channel] = m_channels[channel]->getBuffer()) == nullptr)
				return false;
		}

		for (int channel = 0; channel < 4; channel++) {
			for (int sample = 0; sample < OUTPUT_BUFFER_SAMPLES; sample++){
				float leftValue = (m_control->output2Channels[channel]) ? (m_control->output2Level+1) * channelBuffers[channel][sample] / 8 : 0;
				float rightValue = (m_control->output1Channels[channel]) ? (m_control->output1Level+1) * channelBuffers[channel][sample] / 8 : 0;
				buffer[2*sample] += int16_t(leftValue * 2400);
				buffer[2*sample+1] += int16_t(rightValue * 2400);
			}
		}
		return true;
	}
}
