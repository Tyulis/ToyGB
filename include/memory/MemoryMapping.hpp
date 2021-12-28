#ifndef _MEMORY_MEMORYMAPPING_HPP
#define _MEMORY_MEMORYMAPPING_HPP

#include <cstdint>
#include <iostream>

#include "util/error.hpp"

namespace toygb {
	/** Base class for memory mappings */
	class MemoryMapping {
		public:
			virtual ~MemoryMapping();

			/** Get the value at the given RELATIVE address (relative to the start address configured in the memory map) */
			virtual uint8_t get(uint16_t address) = 0;

			/** Set the value at the given RELATIVE address (relative to the start address configured in the memory map) */
			virtual void set(uint16_t address, uint8_t value) = 0;

			/** Load the memory mapping state from a file (like cartridge RAM save or savestates) */
			virtual void load(std::istream& input);

			/** Save the memory mapping state to a file (like cartridge RAM save or savestates) */
			virtual void save(std::ostream& output);
	};
}

#endif
