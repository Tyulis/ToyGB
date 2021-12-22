#ifndef _AUDIO_MAPPING_AUDIOWAVEMAPPING_HPP
#define _AUDIO_MAPPING_AUDIOWAVEMAPPING_HPP

#include "audio/timing.hpp"
#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "audio/mapping/AudioChannelMapping.hpp"
#include "audio/mapping/AudioControlMapping.hpp"
#include "audio/mapping/WaveMemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioWaveMapping : public AudioChannelMapping {
		public:
			AudioWaveMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, WaveMemoryMapping* wavePatternMapping, HardwareConfig& hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			bool enable;
			uint16_t length;

			uint8_t outputLevel;

			uint16_t frequency;
			bool stopSelect;

		protected:
			float buildSample();
			void reset();
			void onPowerOn();
			void onPowerOff();
			void onUpdate();
			void onLengthFrame();
			void disable();
			void start();

			WaveMemoryMapping* m_wavePatternMapping;

			int m_baseTimerCounter;
			int m_sampleIndex;
	};
}

#endif
