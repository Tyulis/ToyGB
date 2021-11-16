#include "memory/mapping/LCDControlMapping.hpp"

#define OFFSET_START IO_LCD_CONTROL
#define OFFSET_CONTROL     IO_LCD_CONTROL - OFFSET_START
#define OFFSET_STATUS      IO_LCD_STATUS - OFFSET_START
#define OFFSET_SCROLLY     IO_SCROLL_Y - OFFSET_START
#define OFFSET_SCROLLX     IO_SCROLL_X - OFFSET_START
#define OFFSET_COORDY      IO_COORD_Y - OFFSET_START
#define OFFSET_COMPARE     IO_COORD_COMPARE - OFFSET_START

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
		}
	}
}
