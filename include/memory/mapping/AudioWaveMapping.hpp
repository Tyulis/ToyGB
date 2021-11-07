#ifndef _MEMORY_MAPPING_AUDIOWAVEMAPPING_HPP
#define _MEMORY_MAPPING_AUDIOWAVEMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioWaveMapping : public MemoryMapping {
		public:
			AudioWaveMapping();

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			bool enable;
			uint8_t length;

			uint8_t outputLevel;

			uint16_t frequency;
			bool initialize;
			bool stopSelect;
	};
}

#endif
