#ifndef _MEMORY_MAPPING_TIMERMAPPING_HPP
#define _MEMORY_MAPPING_TIMERMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class TimerMapping : public MemoryMapping {
		public:
			TimerMapping();

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			uint8_t divider;
			uint8_t counter;
			uint8_t modulo;
			bool enable;
			uint8_t clockSelect;
	};
}

#endif
