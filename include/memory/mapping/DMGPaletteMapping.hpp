#ifndef _MEMORY_MAPPING_DMGPALETTEMAPPING_HPP
#define _MEMORY_MAPPING_DMGPALETTEMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class DMGPaletteMapping : public MemoryMapping {
		public:
			DMGPaletteMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

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
