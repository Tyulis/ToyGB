#ifndef _MEMORY_MAPPING_AUDIOWAVEMAPPING_HPP
#define _MEMORY_MAPPING_AUDIOWAVEMAPPING_HPP

#include "audio/timing.hpp"
#include "core/OperationMode.hpp"
#include "memory/Constants.hpp"
#include "memory/mapping/AudioChannelMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "memory/mapping/WaveMemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioWaveMapping : public AudioChannelMapping {
		public:
			AudioWaveMapping(int channel, AudioControlMapping* control, WaveMemoryMapping* wavePatternMapping, OperationMode mode);

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
