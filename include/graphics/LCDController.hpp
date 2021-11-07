#ifndef _GRAPHICS_LCDCONTROLLER_HPP
#define _GRAPHICS_LCDCONTROLLER_HPP

#include <queue>
#include <deque>
#include <algorithm>

#include "core/timing.hpp"
#include "core/OperationMode.hpp"
#include "core/InterruptVector.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/HDMAMapping.hpp"
#include "memory/mapping/LCDControlMapping.hpp"
#include "memory/mapping/CGBPaletteMapping.hpp"
#include "memory/mapping/LCDMemoryMapping.hpp"
#include "memory/mapping/LCDBankedMemoryMapping.hpp"
#include "memory/mapping/VRAMBankSelectMapping.hpp"

#define LCD_WIDTH 160
#define LCD_HEIGHT 144


namespace toygb {
	class LCDController {
		public:
			LCDController();
			~LCDController();

			void configureMemory(MemoryMap* memory);

			void init(OperationMode mode, InterruptVector* interrupt);
			void operator()(clocktime_t startTime);

		private:
			void run();
			void dot();
			void dot(int num);

			OperationMode m_mode;
			InterruptVector* m_interrupt;

			HDMAMapping* m_hdma;
			CGBPaletteMapping* m_cgbPalette;
			LCDControlMapping* m_lcdControl;
			LCDMemoryMapping* m_vramMapping;
			LCDMemoryMapping* m_oamMapping;
			VRAMBankSelectMapping* m_vramBankMapping;

			uint8_t* m_vram;
			uint8_t* m_oam;
			uint8_t m_vramBank;

			clocktime_t m_startTime;
			int64_t m_lastCycle;

			uint16_t* m_frontBuffer;
			uint16_t* m_backBuffer;

			class ObjectSelectionComparator {
				public:
					ObjectSelectionComparator(OperationMode mode, LCDMemoryMapping* oam);
					bool operator()(const uint16_t& address1, const uint16_t& address2);

				private:
					OperationMode m_mode;
					LCDMemoryMapping* m_oamMapping;
			};

			class Pixel {
				public:
					Pixel(uint8_t color, uint8_t palette, uint16_t oamAddress, bool priority);

					uint8_t color;
					uint8_t palette;
					uint16_t oamAddress;
					uint8_t priority;
			};
	};
}

#endif
