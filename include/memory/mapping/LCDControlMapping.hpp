#ifndef _MEMORY_MAPPING_LCDCONTROLMAPPING_HPP
#define _MEMORY_MAPPING_LCDCONTROLMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class LCDControlMapping : public MemoryMapping {
		public:
			LCDControlMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			// LCDC
			bool displayEnable;
			bool windowDisplaySelect;
			bool windowEnable;
			bool backgroundDataSelect;
			bool backgroundDisplaySelect;
			bool objectSize;
			bool objectEnable;
			bool backgroundDisplay;

			// STAT
			bool lycInterrupt;
			bool oamInterrupt;
			bool vblankInterrupt;
			bool hblankInterrupt;
			bool coincidenceFlag;
			uint8_t modeFlag;

			// Scroll
			uint8_t scrollX;
			uint8_t scrollY;

			// Coordinate
			uint8_t coordY;
			uint8_t coordYCompare;

			// OAM DMA
			uint16_t oamDMAAddress;
			bool oamDMAActive;

			// Monochrome palettes
			uint8_t backgroundPalette[4];
			uint8_t objectPalette0[4];
			uint8_t objectPalette1[4];

			// Window position
			uint8_t windowY;
			uint8_t windowX;

	};
}

#endif
