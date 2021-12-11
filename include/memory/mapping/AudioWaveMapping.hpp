#ifndef _MEMORY_MAPPING_AUDIOWAVEMAPPING_HPP
#define _MEMORY_MAPPING_AUDIOWAVEMAPPING_HPP

#include "audio/timing.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioWaveMapping : public MemoryMapping {
		public:
			AudioWaveMapping(int channel, AudioControlMapping* control);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			bool enable;
			uint8_t length;

			uint8_t outputLevel;

			uint16_t frequency;
			bool stopSelect;

			bool started;
			uint64_t dotCounter;

			void update();
			int16_t* getBuffer();

		private:
			int m_channel;
			AudioControlMapping* m_control;
	};
}

#endif
