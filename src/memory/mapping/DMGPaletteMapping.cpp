#include "memory/mapping/DMGPaletteMapping.hpp"

#define OFFSET_START IO_BG_PALETTE
#define OFFSET_BGPALETTE   IO_BG_PALETTE - OFFSET_START
#define OFFSET_OBJ0PALETTE IO_OBJ0_PALETTE - OFFSET_START
#define OFFSET_OBJ1PALETTE IO_OBJ1_PALETTE - OFFSET_START
#define OFFSET_WINDOWY     IO_WINDOW_Y - OFFSET_START
#define OFFSET_WINDOWX     IO_WINDOW_X - OFFSET_START

namespace toygb {
	DMGPaletteMapping::DMGPaletteMapping() {
		// Monochrome palettes
		backgroundPalette[0] = 3;
		backgroundPalette[1] = 3;
		backgroundPalette[2] = 2;
		backgroundPalette[3] = 0;

		objectPalette0[0] = 3;
		objectPalette0[1] = 3;
		objectPalette0[2] = 3;
		objectPalette0[3] = 3;

		objectPalette1[0] = 3;
		objectPalette1[1] = 3;
		objectPalette1[2] = 3;
		objectPalette1[3] = 3;

		windowX = 0x00;
		windowY = 0x00;
	}

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
