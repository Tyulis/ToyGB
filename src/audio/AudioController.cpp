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

		m_wavePatternMapping = new ArrayMemoryMapping(m_wavePattern);
		m_control = new AudioControlMapping();
		m_channels[0] = new AudioToneSweepMapping(0, m_control);
		m_channels[1] = new AudioToneMapping(1, m_control);
		m_channels[2] = new AudioWaveMapping(2, m_control, m_wavePatternMapping);
		m_channels[3] = new AudioNoiseMapping(3, m_control);
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
			for (int channel = 0; channel < 4; channel++){
				if (m_control->audioEnable){
					m_channels[channel]->update();
				} else {
					m_channels[channel]->disable();
				}
			}
			dot();
		}
	}

	int16_t* AudioController::getSamples(int channel){
		return m_channels[channel]->getBuffer();
	}
}
