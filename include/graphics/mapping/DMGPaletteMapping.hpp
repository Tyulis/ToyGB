#ifndef _GRAPHICS_MAPPING_DMGPALETTEMAPPING_HPP
#define _GRAPHICS_MAPPING_DMGPALETTEMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** DMG palettes and window position (as it is contiguous...) IO registers memory mapping */
	class DMGPaletteMapping : public MemoryMapping {
		public:
			DMGPaletteMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			// Monochrome palettes
			uint8_t backgroundPalette[4];  // 2-bits colors for each 2-bits palette index (accessible from register BGP)
			uint8_t objectPalette0[4];     // Same for object palette 0 (accessible from register OBP0)
			uint8_t objectPalette1[4];     // Same for object palette 1 (accessible from register OBP1)

			// Window position
			uint8_t windowY;  // Position of the left of the window on the screen
			uint8_t windowX;  // Position of the top of the window on the screen

	};
}

#endif
