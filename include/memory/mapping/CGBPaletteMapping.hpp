#ifndef _MEMORY_MAPPING_CGBPALETTEMAPPING_HPP
#define _MEMORY_MAPPING_CGBPALETTEMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class CGBPaletteMapping : public MemoryMapping {
		public:
			CGBPaletteMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			bool accessible;

			bool backgroundAutoIncrement;
			uint8_t backgroundIndex;
			uint16_t backgroundPalettes[8*4];

			bool objectAutoIncrement;
			uint8_t objectIndex;
			uint16_t objectPalettes[8*4];
	};
}

#endif
