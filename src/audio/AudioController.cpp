#include "audio/AudioController.hpp"


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
		m_channel1 = new AudioToneSweepMapping();
		m_channel2 = new AudioToneMapping();
		m_channel3 = new AudioWaveMapping();
		m_channel4 = new AudioNoiseMapping();
		m_control = new AudioControlMapping();
	}

	void AudioController::configureMemory(MemoryMap* memory) {
		memory->add(IO_CH1_SWEEP, IO_CH1_CONTROL, m_channel1);
		memory->add(IO_CH2_PATTERN, IO_CH2_CONTROL, m_channel2);
		memory->add(IO_CH3_ENABLE, IO_CH3_CONTROL, m_channel3);
		memory->add(IO_CH4_LENGTH, IO_CH4_CONTROL, m_channel4);
		memory->add(IO_AUDIO_LEVELS, IO_AUDIO_ENABLE, m_control);
		memory->add(IO_WAVEPATTERN_START, IO_WAVEPATTERN_END, m_wavePatternMapping);
	}

	void AudioController::operator()() {
		
	}
}
