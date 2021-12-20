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
#include "memory/mapping/AudioDebugMapping.hpp"
#include "memory/mapping/WaveMemoryMapping.hpp"
#include "util/component.hpp"


namespace toygb {
	/** Main audio controller, emulates the APU */
	class AudioController {
		public:
			AudioController();
			~AudioController();

			void configureMemory(MemoryMap* memory);
			void init(HardwareConfig& hardware);

			GBComponent run();

			/** Read the samples for an audio buffer if available
			 * If the amount of available samples is greater than audio/timing.hpp:OUTPUT_BUFFER_SAMPLES,
			 * return true and fill the given buffer with the available samples
			 * Otherwise, return false and clear the buffer.
			 * The output is the fully mixed PCM16 audio data */
			bool getSamples(int16_t* buffer);

		private:
			HardwareConfig m_hardware;

			uint8_t* m_wavePattern;

			AudioChannelMapping* m_channels[4];
			AudioControlMapping* m_control;
			AudioDebugMapping* m_debug;
			WaveMemoryMapping* m_wavePatternMapping;
	};
}

#endif
