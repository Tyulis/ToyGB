#ifndef _GRAPHICS_MAPPING_LCDMEMORYMAPPING_HPP
#define _GRAPHICS_MAPPING_LCDMEMORYMAPPING_HPP

#include "memory/MemoryMapping.hpp"

namespace toygb {
	/** Base class for array memories with a specific PPU access */
	class LCDMemoryMapping : public MemoryMapping {
		public:
			LCDMemoryMapping(uint8_t* array);

			// CPU access : unavailable while the PPU is accessing
			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			// PPU access : unavailable when the PPU has not reserved it
			virtual uint8_t lcdGet(uint16_t address);
			virtual void lcdSet(uint16_t address, uint8_t value);

			bool accessible;  // Whether the mapping is accessible to the CPU

		protected:
			uint8_t* m_array;
	};
}

#endif
