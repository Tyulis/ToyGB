#include "audio/AudioController.hpp"

#define CHANNEL_TONE_SWEEP 0
#define CHANNEL_TONE 1
#define CHANNEL_WAVE 2
#define CHANNEL_NOISE 3


#define SAMPLE_LOW (-2000)
#define SAMPLE_HIGH (2000)

namespace toygb {
	AudioController::AudioController() {
		m_channel1 = nullptr;
		m_channel2 = nullptr;
		m_channel3 = nullptr;
		m_channel4 = nullptr;
		m_control = nullptr;
		m_wavePatternMapping = nullptr;
		m_wavePattern = nullptr;
	}

	AudioController::~AudioController() {
		if (m_wavePattern != nullptr) delete[] m_wavePattern;
		if (m_wavePatternMapping != nullptr) delete m_wavePatternMapping;
		if (m_channel1 != nullptr) delete m_channel1;
		if (m_channel2 != nullptr) delete m_channel2;
		if (m_channel3 != nullptr) delete m_channel3;
		if (m_channel4 != nullptr) delete m_channel4;
		if (m_control != nullptr) delete m_control;
	}

	void AudioController::init(OperationMode mode){
		m_mode = mode;
		m_wavePattern = new uint8_t[IO_WAVEPATTERN_SIZE];

		m_wavePatternMapping = new ArrayMemoryMapping(m_wavePattern);
		m_control = new AudioControlMapping();
		m_channel1 = new AudioToneSweepMapping(0, m_control);
		m_channel2 = new AudioToneMapping(1, m_control);
		m_channel3 = new AudioWaveMapping(2, m_control);
		m_channel4 = new AudioNoiseMapping(3, m_control);
	}

	void AudioController::configureMemory(MemoryMap* memory) {
		memory->add(IO_CH1_SWEEP, IO_CH1_CONTROL, m_channel1);
		memory->add(IO_CH2_PATTERN, IO_CH2_CONTROL, m_channel2);
		memory->add(IO_CH3_ENABLE, IO_CH3_CONTROL, m_channel3);
		memory->add(IO_CH4_LENGTH, IO_CH4_CONTROL, m_channel4);
		memory->add(IO_AUDIO_LEVELS, IO_AUDIO_ENABLE, m_control);
		memory->add(IO_WAVEPATTERN_START, IO_WAVEPATTERN_END, m_wavePatternMapping);
	}

#define dot() co_await std::suspend_always()

	GBComponent AudioController::run(){
		while (true){
			if (m_control->audioEnable){
				m_channel1->update();
				m_channel2->update();
				m_channel3->update();
				m_channel4->update();
			} else {
				m_channel1->started = false;
				m_channel2->started = false;
				m_channel3->started = false;
				m_channel4->started = false;
			}
			dot();
		}
	}

	int16_t* AudioController::getSamples(int channel){
		switch (channel){
			case CHANNEL_TONE_SWEEP: return m_channel1->getBuffer();
			case CHANNEL_TONE:       return m_channel2->getBuffer();
			case CHANNEL_WAVE:       return m_channel3->getBuffer();
			case CHANNEL_NOISE:      return m_channel4->getBuffer();
		}
		return nullptr;
	}
}
