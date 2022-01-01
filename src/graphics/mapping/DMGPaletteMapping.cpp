#include "graphics/mapping/DMGPaletteMapping.hpp"

#define OFFSET_START IO_BG_PALETTE
#define OFFSET_BGPALETTE   IO_BG_PALETTE - OFFSET_START
#define OFFSET_OBJ0PALETTE IO_OBJ0_PALETTE - OFFSET_START
#define OFFSET_OBJ1PALETTE IO_OBJ1_PALETTE - OFFSET_START
#define OFFSET_WINDOWY     IO_WINDOW_Y - OFFSET_START
#define OFFSET_WINDOWX     IO_WINDOW_X - OFFSET_START

/** DMG momochrome palettes IO registers mapping
Every palette has the same structure : 33221100
- 3 (bits 6-7) : Color for color index 3
- 2 (bits 4-5) : Color for color index 2
- 1 (bits 2-3) : Color for color index 1
- 0 (bits 0-1) : Color for color index 0
For object palettes (OBP0 and OBP1), color index 0 is always transparent, so the two lower bits can store a value but are unused

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF47 |       0000 |  BGP | BBBBBBBB | Monochrome palette for background tiles
      FF48 |       0001 | OBP0 | BBBBBBBB | Monochrome palette for objects with palette 0 set in their OAM control byte
      FF49 |       0002 | OBP1 | BBBBBBBB | Monochrome palette for objects with palette 1 set in their OAM control byte */


namespace toygb {
	// Initialize the memory mapping with its initial values
	DMGPaletteMapping::DMGPaletteMapping() {
		
	}

	// Get the value at the given relative address
	uint8_t DMGPaletteMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_BGPALETTE:
				return (
					(backgroundPalette[3] << 6) |
					(backgroundPalette[2] << 4) |
					(backgroundPalette[1] << 2) |
					backgroundPalette[0]);
			case OFFSET_OBJ0PALETTE:
				return (
					(objectPalette0[3] << 6) |
					(objectPalette0[2] << 4) |
					(objectPalette0[1] << 2) |
					objectPalette0[0]);
			case OFFSET_OBJ1PALETTE:
				return (
					(objectPalette1[3] << 6) |
					(objectPalette1[2] << 4) |
					(objectPalette1[1] << 2) |
					objectPalette1[0]);
			case OFFSET_WINDOWY: return windowY;
			case OFFSET_WINDOWX: return windowX;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	// Set the value at the given memory address
	void DMGPaletteMapping::set(uint16_t address, uint8_t value) {
		switch (address) {
			case OFFSET_BGPALETTE:
				backgroundPalette[0] = value & 3;
				backgroundPalette[1] = (value >> 2) & 3;
				backgroundPalette[2] = (value >> 4) & 3;
				backgroundPalette[3] = (value >> 6) & 3;
				break;
			case OFFSET_OBJ0PALETTE:
				objectPalette0[0] = value & 3;
				objectPalette0[1] = (value >> 2) & 3;
				objectPalette0[2] = (value >> 4) & 3;
				objectPalette0[3] = (value >> 6) & 3;
				break;
			case OFFSET_OBJ1PALETTE:
				objectPalette1[0] = value & 3;
				objectPalette1[1] = (value >> 2) & 3;
				objectPalette1[2] = (value >> 4) & 3;
				objectPalette1[3] = (value >> 6) & 3;
				break;
			case OFFSET_WINDOWY: windowY = value; break;
			case OFFSET_WINDOWX: windowX = value; break;
		}
	}
}
