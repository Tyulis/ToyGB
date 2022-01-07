#ifndef _GRAPHICS_LCDCONTROLLER_HPP
#define _GRAPHICS_LCDCONTROLLER_HPP

#include <queue>
#include <deque>
#include <algorithm>

#include "core/timing.hpp"
#include "core/hardware.hpp"
#include "core/InterruptVector.hpp"
#include "graphics/mapping/OAMMapping.hpp"
#include "graphics/mapping/LCDControlMapping.hpp"
#include "graphics/mapping/DMGPaletteMapping.hpp"
#include "graphics/mapping/CGBPaletteMapping.hpp"
#include "graphics/mapping/LCDMemoryMapping.hpp"
#include "graphics/mapping/LCDBankedMemoryMapping.hpp"
#include "graphics/mapping/VRAMBankSelectMapping.hpp"
#include "memory/MemoryMap.hpp"
#include "util/component.hpp"

// Screen dimensions
#define LCD_WIDTH 160
#define LCD_HEIGHT 144


namespace toygb {
	/** Implementation of the Picture Processing Unit and LCD screen */
	class LCDController {
		public:
			LCDController();
			~LCDController();

			void init(HardwareStatus* hardware, InterruptVector* interrupt);
			void configureMemory(MemoryMap* memory);

			/** Main loop of the component, as a coroutine */
			GBComponent run();

			/** Tell whether the emulator can skip the component on that cycle, to save a context commutation */
			bool skip();

			uint16_t* pixels();  // Return the full pixels buffer, as a CGB RGB555 bitmap (even in DMG mode)

		private:
			/** Comparator for sprite rendering order */
			class ObjectSelectionComparator {
				public:
					ObjectSelectionComparator(HardwareStatus* hardware, LCDMemoryMapping* oam);
					bool operator()(const uint16_t& address1, const uint16_t& address2);

				private:
					HardwareStatus* m_hardware;
					LCDMemoryMapping* m_oamMapping;
			};

			/** Pixel information, as pushed onto a pixel FIFO */
			class Pixel {
				public:
					Pixel(uint8_t color, uint8_t palette, uint16_t oamAddress, bool priority);

					uint8_t color;        // 2-bits color value
					uint8_t palette;      // Palette to use
					uint16_t oamAddress;  // Address of the object in OAM
					uint8_t priority;     // Pixel priority value
			};

			HardwareStatus* m_hardware;
			InterruptVector* m_interrupt;

			// Related memory mappings
			CGBPaletteMapping* m_cgbPalette;
			LCDControlMapping* m_lcdControl;
			DMGPaletteMapping* m_dmgPalette;
			LCDMemoryMapping* m_vramMapping;
			LCDMemoryMapping* m_oamMapping;
			VRAMBankSelectMapping* m_vramBankMapping;

			// Array memory data
			uint8_t* m_vram;
			uint8_t* m_oam;
			uint8_t m_vramBank;

			// The whole thing is double-buffered
			uint16_t* m_frontBuffer;
			uint16_t* m_backBuffer;

			int m_cyclesToSkip;
	};
}

#endif
