#ifndef _MEMORY_MEMORYMAPPING_HPP
#define _MEMORY_MEMORYMAPPING_HPP

#include <cstdint>
#include <iostream>

#include "util/error.hpp"

namespace toygb {
	class MemoryMapping {
		public:
			virtual ~MemoryMapping();

			virtual uint8_t get(uint16_t address) = 0;
			virtual void set(uint16_t address, uint8_t value) = 0;

			virtual void load(std::istream& input);
			virtual void save(std::ostream& output);
	};
}

#endif
