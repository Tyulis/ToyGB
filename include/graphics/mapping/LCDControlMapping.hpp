#ifndef _GRAPHICS_MAPPING_LCDCONTROLMAPPING_HPP
#define _GRAPHICS_MAPPING_LCDCONTROLMAPPING_HPP

#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** LCD control IO registers memory mapping */
	class LCDControlMapping : public MemoryMapping {
		public:
			LCDControlMapping(HardwareStatus* hardware);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			// LCDC : LCD control
			bool displayEnable;            // Whether the PPU is active and the display active (register LCDC, bit 7)
			bool windowTilemapSelect;      // Window tilemap position (0 = 0x9800, 1 = 0x9C00) (register LCDC, bit 6)
			bool windowEnable;             // Enable window rendering (0 = disabled, 1 = enabled) (register LCDC, bit 5)
			bool backgroundDataSelect;     // Background and window tile data position (0 = 0x8800, 1 = 0x8000) (register LCDC, bit 4)
			bool backgroundTilemapSelect;  // Background tilemap position (0 = 0x9800, 1 = 0x9C00) (register LCDC, bit 3)
			bool objectSize;               // Object size (0 = 8x8, 1 = 8x16) (register LCDC, bit 2)
			bool objectEnable;             // Display objects (0 = disabled, 1 = enabled) (register LCDC, bit 1)
			bool backgroundDisplay;        // Background and window enable (DMG) / priority (CGB) (0 = disabled, 1 = enabled) (register LCDC, bit 0)

			// STAT : PPU status register
			bool lycInterrupt;     // Request a STAT interrupt when LY = LYC (register STAT, bit 6)
			bool oamInterrupt;     // Request a STAT interrupt when PPU enters mode 2 (OAM scan) (register STAT, bit 5)
			bool vblankInterrupt;  // Request a STAT interrupt when PPU enters mode 1 (VBlank) (register STAT, bit 4)
			bool hblankInterrupt;  // Request a STAT interrupt when PPU enters mode 0 (HBlank) (register STAT, bit 3)
			uint8_t modeFlag;      // Current PPU mode (0 = HBlank, 1 = VBlank, 2 = OAM scan, 3 = rendering)

			// Background scrolling
			uint8_t scrollX;  // Left position of the screen in the 256*256px background tilemap (register SCX)
			uint8_t scrollY;  // Top position of the screen in the 256*256px background tilemap (register SCY)

			// Coordinate
			uint8_t coordY;         // Current scanline being rendered (register LY)
			uint8_t coordYCompare;  // Value the scanline must be compared against (register LYC)

		private:
			void shutdownPPU();  // Called when the PPU is shut down (LCDC.7 goes 1 -> 0)

			HardwareStatus* m_hardware;
	};
}

#endif
