#ifndef _AUDIO_AUDIOCONTROLLER_HPP
#define _AUDIO_AUDIOCONTROLLER_HPP

#include "core/timing.hpp"
#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/AudioChannelMapping.hpp"
#include "memory/mapping/AudioToneSweepMapping.hpp"
#include "memory/mapping/AudioToneMapping.hpp"
#include "memory/mapping/AudioWaveMapping.hpp"
#include "memory/mapping/AudioNoiseMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "memory/mapping/WaveMemoryMapping.hpp"
#include "util/component.hpp"


namespace toygb {
	class AudioController {
		public:
			AudioController();
			~AudioController();

			void configureMemory(MemoryMap* memory);
			void init(HardwareConfig& hardware);

			GBComponent run();

			bool getSamples(int16_t* buffer);

		private:
			HardwareConfig m_hardware;

			uint8_t* m_wavePattern;

			AudioChannelMapping* m_channels[4];
			AudioControlMapping* m_control;
			WaveMemoryMapping* m_wavePatternMapping;
	};
}

#endif
