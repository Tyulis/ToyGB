#ifndef _GRAPHICS_MAPPING_CGBPALETTEMAPPING_HPP
#define _GRAPHICS_MAPPING_CGBPALETTEMAPPING_HPP

#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** CGB palettes IO registers memory mapping */
	class CGBPaletteMapping : public MemoryMapping {
		public:
			CGBPaletteMapping(HardwareConfig* hardware);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			bool accessible;  // Whether the mapping is accessible to the CPU (i.e if the PPU is not accessing it)

			// Background palettes
			bool backgroundAutoIncrement;      // Whether to auto-increment the current accessed index in the background palettes after writing (register BCPS, bit 7)
			uint8_t backgroundIndex;           // Currently accessible index from background palette data register (register BCPS, bits 0-5)
			uint16_t backgroundPalettes[8*4];  // Background palettes content (accessible from register BCPD)

			// Object palettes
			bool objectAutoIncrement;          // Whether to auto-increment the current accessed index in the object palettes after writing (register OCPS, bit 7)
			uint8_t objectIndex;               // Currently accessible index from object palette data register (register BCPS, bits 0-5)
			uint16_t objectPalettes[8*4];      // Object palettes content (accessible from register OCPD)

			bool objectPriority;               // Object priority mode (0 = CGB mode, priority to the lowest OAM index, 1 = DMG mode, priority to the lowest X coordinate) (register OPRI, bit 0)

		private:
			HardwareConfig* m_hardware;
	};
}

#endif
