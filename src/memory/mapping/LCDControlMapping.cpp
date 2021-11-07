#include "memory/mapping/LCDControlMapping.hpp"

#define OFFSET_START IO_LCD_CONTROL
#define OFFSET_CONTROL     IO_LCD_CONTROL - OFFSET_START
#define OFFSET_STATUS      IO_LCD_STATUS - OFFSET_START
#define OFFSET_SCROLLY     IO_SCROLL_Y - OFFSET_START
#define OFFSET_SCROLLX     IO_SCROLL_X - OFFSET_START
#define OFFSET_COORDY      IO_COORD_Y - OFFSET_START
#define OFFSET_COMPARE     IO_COORD_COMPARE - OFFSET_START
#define OFFSET_OAMDMA      IO_OAM_DMA - OFFSET_START
#define OFFSET_BGPALETTE   IO_BG_PALETTE - OFFSET_START
#define OFFSET_OBJ0PALETTE IO_OBJ0_PALETTE - OFFSET_START
#define OFFSET_OBJ1PALETTE IO_OBJ1_PALETTE - OFFSET_START
#define OFFSET_WINDOWY     IO_WINDOW_Y - OFFSET_START
#define OFFSET_WINDOWX     IO_WINDOW_X - OFFSET_START

namespace toygb {
	LCDControlMapping::LCDControlMapping() {
		// LCDC
		displayEnable = true;
		windowDisplaySelect = false;
		windowEnable = false;
		backgroundDataSelect = true;
		backgroundDisplaySelect = false;
		objectSize = false;
		objectEnable = false;
		backgroundDisplay = true;

		// STAT
		lycInterrupt = false;
		oamInterrupt = false;
		vblankInterrupt = false;
		hblankInterrupt = false;
		coincidenceFlag = false;
		modeFlag = 0;

		// Scroll
		scrollY = 0x00;
		scrollX = 0x00;

		// Coordinate
		coordY = 0x00;
		coordYCompare = 0x00;

		// OAM DMA
		oamDMAAddress = 0x00;
		oamDMAActive = false;

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

	uint8_t LCDControlMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_CONTROL:
				return (
					(displayEnable << 7) |
					(windowDisplaySelect << 6) |
					(windowEnable << 5) |
					(backgroundDataSelect << 4) |
					(backgroundDisplaySelect << 3) |
					(objectSize << 2) |
					(objectEnable << 1) |
					backgroundDisplay);
			case OFFSET_STATUS:
				return (
					(lycInterrupt << 6) |
					(oamInterrupt << 5) |
					(vblankInterrupt << 4) |
					(hblankInterrupt << 3) |
					(coincidenceFlag << 2) |
					modeFlag);
			case OFFSET_SCROLLY: return scrollY;
			case OFFSET_SCROLLX: return scrollX;
			case OFFSET_COORDY: return coordY;
			case OFFSET_COMPARE: return coordYCompare;
			case OFFSET_OAMDMA: return 0x00;  // FIXME : read write-only
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

	void LCDControlMapping::set(uint16_t address, uint8_t value) {
		switch (address) {
			case OFFSET_CONTROL:
				displayEnable = (value >> 7) & 1;
				windowDisplaySelect = (value >> 6) & 1;
				windowEnable = (value >> 5) & 1;
				backgroundDataSelect = (value >> 4) & 1;
				backgroundDisplaySelect = (value >> 3) & 1;
				objectSize = (value >> 2) & 1;
				objectEnable = (value >> 1) & 1;
				backgroundDisplay = value & 1;
				break;
			case OFFSET_STATUS:
				lycInterrupt = (value >> 6) & 1;
				oamInterrupt = (value >> 5) & 1;
				vblankInterrupt = (value >> 4) & 1;
				hblankInterrupt = (value >> 3) & 1;
				break;
			case OFFSET_SCROLLY: scrollY = value; break;
			case OFFSET_SCROLLX: scrollX = value; break;
			// case OFFSET_COORDY
			case OFFSET_COMPARE: coordYCompare = value; break;
			case OFFSET_OAMDMA:
				oamDMAActive = true;
				oamDMAAddress = value << 8;
				break;
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
