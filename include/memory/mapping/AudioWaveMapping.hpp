#ifndef _MEMORY_MAPPING_AUDIOWAVEMAPPING_HPP
#define _MEMORY_MAPPING_AUDIOWAVEMAPPING_HPP

#include "audio/timing.hpp"
#include "memory/Constants.hpp"
#include "memory/mapping/AudioChannelMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioWaveMapping : public AudioChannelMapping {
		public:
			AudioWaveMapping(int channel, AudioControlMapping* control, ArrayMemoryMapping* wavePatternMapping);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			bool enable;
			uint8_t length;

			uint8_t outputLevel;

			uint16_t frequency;
			bool stopSelect;

			void update();

		private:
			int16_t buildSample();
			void reset();

			ArrayMemoryMapping* m_wavePatternMapping;

			int m_lengthTimerCounter;
			int m_baseTimerCounter;
			int m_outputTimerCounter;
			int m_sampleIndex;
	};
}

#endif
