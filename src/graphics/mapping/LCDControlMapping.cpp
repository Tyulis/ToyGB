#include "graphics/mapping/LCDControlMapping.hpp"

#define OFFSET_START IO_LCD_CONTROL
#define OFFSET_CONTROL     IO_LCD_CONTROL - OFFSET_START
#define OFFSET_STATUS      IO_LCD_STATUS - OFFSET_START
#define OFFSET_SCROLLY     IO_SCROLL_Y - OFFSET_START
#define OFFSET_SCROLLX     IO_SCROLL_X - OFFSET_START
#define OFFSET_COORDY      IO_COORD_Y - OFFSET_START
#define OFFSET_COMPARE     IO_COORD_COMPARE - OFFSET_START

/** PPU control IO registers mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF40 |       0000 | LCDC | BBBBBBBB | Main LCD control register : 76543210
           |            |      |          | - Bit 7 : LCD and PPU enable (0 = stop all PPU operation and shut the LCD down, 1 = enable)
           |            |      |          | - Bit 6 : Window tile map selection (0 = 0x9800-0x9BFF, 1 = 0x9C00-0x9FFF)
           |            |      |          | - Bit 5 : Window enable (0 = disabled, 1 = enabled)
           |            |      |          | - Bit 4 : Background / window tile data selection (0 = 0x9000-0x97FF + 0x8800-0x8FFF, 1 = 0x8000-0x8FFF)
           |            |      |          | - Bit 3 : Background tile map selection (0 = 0x9800-0x9BFF, 1 = 0x9C00-0x9FFF)
           |            |      |          | - Bit 2 : Objects size (0 = 8x8 = 1 tile, 1 = 8x16 = 2 tiles stacked vertically)
           |            |      |          | - Bit 1 : Objects enable (0 = disabled, 1 = enabled)
           |            |      |          | - Bit 0 : Background / window enable / priority
           |            |      |          |           In DMG mode, 0 = background and window disabled (blank background, only sprites displayed), 1 = background and window enabled
           |            |      |          |           In CGB mode, 0 = objects have priority over background and window regardless of the priority bits in OAM and background attribute map,
           |            |      |          |                        1 = background-to-object priority is handled by the background attribute priority bit, then OAM priority bit (as normal)
      FF41 |       0001 | STAT | -BBBBRRR | LCD status register and STAT interrupt control : -C210FMM
           |            |      |          | - C (bit 6) : 1 = request a STAT interrupt when LY = LYC, 0 = don't
           |            |      |          | - 2 (bit 5) : 1 = request a STAT interrupt when entering PPU mode 2 (OAM scan), 0 = don't
           |            |      |          | - 1 (bit 4) : 1 = request a STAT interrupt when entering PPU mode 1 (VBlank), 0 = don't
           |            |      |          | - 0 (bit 3) : 1 = request a STAT interrupt when entering PPU mode 0 (HBlank), 0 = don't
           |            |      |          | - F (bit 2) : LY = LYC coincidence flag (reads 1 if LY = LYC, 0 otherwise)
           |            |      |          | - M (bits 0-1) : Reads the current PPU mode (0 = HBlank, 1 = VBlank, 2 = OAM scan, 3 = rendering)
      FF42 |       0002 |  SCY | BBBBBBBB | Background Y scrolling, specify the X coordinate (in pixels) of the 160*144px screen area within the 256*256px background area
      FF43 |       0003 |  SCX | BBBBBBBB | Background X scrolling, specify the Y coordinate of the screen area within the background area
      FF44 |       0004 |   LY | RRRRRRRR | Reads the index of the scanline that is currently being rendered by the PPU
      FF45 |       0005 |  LYC | BBBBBBBB | Index the current scanline in LY must be compared against to set STAT.2 and trigger the STAT interrupt if STAT.6 is set */


namespace toygb {
	// Initialize the memory mapping with initial values
	LCDControlMapping::LCDControlMapping(HardwareStatus* hardware) {
		m_hardware = hardware;

		// LCDC
		displayEnable = !m_hardware->hasBootrom();  // The gameboy apparently stats with the LCD disabled, it is enabled later by the bootrom
		windowTilemapSelect = false;
		windowEnable = false;
		backgroundDataSelect = true;
		backgroundTilemapSelect = false;
		objectSize = false;
		objectEnable = false;
		backgroundDisplay = true;

		// STAT
		lycInterrupt = false;
		oamInterrupt = false;
		vblankInterrupt = false;
		hblankInterrupt = false;
		modeFlag = 0;

		// Scrolling
		scrollY = 0x00;
		scrollX = 0x00;

		// Coordinate
		coordY = 0x00;
		coordYCompare = 0x00;
	}

	// Get the value at the given relative address
	uint8_t LCDControlMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_CONTROL:  // LCDC
				return (
					(displayEnable << 7) |
					(windowTilemapSelect << 6) |
					(windowEnable << 5) |
					(backgroundDataSelect << 4) |
					(backgroundTilemapSelect << 3) |
					(objectSize << 2) |
					(objectEnable << 1) |
					backgroundDisplay);
			case OFFSET_STATUS:  // STAT
				return (
					(lycInterrupt << 6) |
					(oamInterrupt << 5) |
					(vblankInterrupt << 4) |
					(hblankInterrupt << 3) |
					((coordY == coordYCompare) << 2) |
					modeFlag | 0x80);
			case OFFSET_SCROLLY:  // SCY
				return scrollY;
			case OFFSET_SCROLLX:  // SCX
				return scrollX;
			case OFFSET_COORDY:   // LY
				return coordY;
			case OFFSET_COMPARE:  // LYC
				return coordYCompare;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	// Set the value at the given relative address
	void LCDControlMapping::set(uint16_t address, uint8_t value) {
		switch (address) {
			case OFFSET_CONTROL:  // LCDC
				if (displayEnable && ((value >> 7) & 1))
					shutdownPPU();
				displayEnable = (value >> 7) & 1;
				windowTilemapSelect = (value >> 6) & 1;
				windowEnable = (value >> 5) & 1;
				backgroundDataSelect = (value >> 4) & 1;
				backgroundTilemapSelect = (value >> 3) & 1;
				objectSize = (value >> 2) & 1;
				objectEnable = (value >> 1) & 1;
				backgroundDisplay = value & 1;
				break;
			case OFFSET_STATUS:  // STAT
				lycInterrupt = (value >> 6) & 1;
				oamInterrupt = (value >> 5) & 1;
				vblankInterrupt = (value >> 4) & 1;
				hblankInterrupt = (value >> 3) & 1;
				break;
			case OFFSET_SCROLLY:  // SCY
				scrollY = value;
				break;
			case OFFSET_SCROLLX:  // SCX
				scrollX = value;
				break;
			// case OFFSET_COORDY
			case OFFSET_COMPARE:  // LYC
				coordYCompare = value;
				break;
		}
	}

	// Perform status changes when the PPU is shut down using LCDC.7
	void LCDControlMapping::shutdownPPU() {
		coordY = 0;
		modeFlag = 0;
	}
}
