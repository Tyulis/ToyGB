#ifndef _AUDIO_AUDIOCONTROLLER_HPP
#define _AUDIO_AUDIOCONTROLLER_HPP

#include "core/OperationMode.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/AudioToneSweepMapping.hpp"
#include "memory/mapping/AudioToneMapping.hpp"
#include "memory/mapping/AudioWaveMapping.hpp"
#include "memory/mapping/AudioNoiseMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"


namespace toygb {
	class AudioController {
		public:
			AudioController();
			~AudioController();

			void configureMemory(MemoryMap* memory);

			void init(OperationMode mode);
			void operator()();

		private:
			OperationMode m_mode;

			uint8_t* m_wavePattern;

			AudioToneSweepMapping* m_channel1;
			AudioToneMapping* m_channel2;
			AudioWaveMapping* m_channel3;
			AudioNoiseMapping* m_channel4;
			AudioControlMapping* m_control;
			ArrayMemoryMapping* m_wavePatternMapping;
	};
}

#endif
