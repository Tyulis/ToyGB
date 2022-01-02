#ifndef _AUDIO_AUDIOCONTROLLER_HPP
#define _AUDIO_AUDIOCONTROLLER_HPP

// Reference for almost everything in the audio controller : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware

#include "audio/mapping/AudioChannelMapping.hpp"
#include "audio/mapping/AudioToneSweepMapping.hpp"
#include "audio/mapping/AudioToneMapping.hpp"
#include "audio/mapping/AudioWaveMapping.hpp"
#include "audio/mapping/AudioNoiseMapping.hpp"
#include "audio/mapping/AudioControlMapping.hpp"
#include "audio/mapping/AudioDebugMapping.hpp"
#include "audio/mapping/WaveMemoryMapping.hpp"
#include "core/timing.hpp"
#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "util/component.hpp"


namespace toygb {
	/** Main audio controller, emulates the APU */
	class AudioController {
		public:
			AudioController();
			~AudioController();

			void configureMemory(MemoryMap* memory);
			void init(HardwareStatus* hardware);

			/** Main loop of the component, as a coroutine */
			GBComponent run();

			/** Tell whether the emulator can skip the component on that cycle, to save a context commutation */
			bool skip();

			/** Read the samples for an audio buffer if available
			 * If the amount of available samples is greater than audio/timing.hpp:OUTPUT_BUFFER_SAMPLES,
			 * return true and fill the given buffer with the available samples
			 * Otherwise, return false and clear the buffer.
			 * The output is the fully mixed PCM16 audio data */
			bool getSamples(int16_t* buffer);

		private:
			HardwareStatus* m_hardware;

			uint8_t* m_wavePattern;  // Wave RAM

			AudioChannelMapping* m_channels[4];
			AudioControlMapping* m_control;
			AudioDebugMapping* m_debug;
			WaveMemoryMapping* m_wavePatternMapping;

			int m_cyclesToSkip;
	};
}

#endif
