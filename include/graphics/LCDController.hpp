#ifndef _GRAPHICS_LCDCONTROLLER_HPP
#define _GRAPHICS_LCDCONTROLLER_HPP

#include <queue>
#include <deque>
#include <algorithm>

#include "util/component.hpp"
#include "core/timing.hpp"
#include "core/OperationMode.hpp"
#include "core/InterruptVector.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/OAMMapping.hpp"
#include "memory/mapping/HDMAMapping.hpp"
#include "memory/mapping/LCDControlMapping.hpp"
#include "memory/mapping/DMGPaletteMapping.hpp"
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
			GBComponent run();

			uint16_t* pixels();

		private:
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

			enum class SpriteFetcherStep {
				Idle,
				AwaitBackgroundFetcher,
				FetchLow, FetchHigh,
			};

			void dotOperation();
			void oamScanDot(int line, int dot);
			void pixelDrawingDot(int line, int dot);
			void hblankDot(int line, int dot);
			void vblankDot(int line, int dot);
			void runBackgroundPixelFetcher(int line, int dot);
			bool runObjectFetcher(int line, int dot);
			void renderPixel(int line, int dot);

			OperationMode m_mode;
			InterruptVector* m_interrupt;

			HDMAMapping* m_hdma;
			CGBPaletteMapping* m_cgbPalette;
			LCDControlMapping* m_lcdControl;
			DMGPaletteMapping* m_dmgPalette;
			LCDMemoryMapping* m_vramMapping;
			LCDMemoryMapping* m_oamMapping;
			VRAMBankSelectMapping* m_vramBankMapping;

			uint8_t* m_vram;
			uint8_t* m_oam;
			uint8_t m_vramBank;

			uint16_t* m_frontBuffer;
			uint16_t* m_backBuffer;

			// Dot operation
			int m_dotCounter;

			// Mode 2
			uint16_t m2_oamAddress;
			uint8_t m2_yValue;
			std::deque<uint16_t> m2_selectedSprites;

			// Mode 3
			std::deque<Pixel> m3_backgroundQueue;
			std::deque<Pixel> m3_objectQueue;
			int m3_xPos;
			int m3_fetcherStep;
			int m3_fetcherLoopDot;
			uint8_t m3_tileIndex;
			uint16_t m3_tileAddress;
			uint8_t m3_indexY;
			uint8_t m3_tileLow;
			uint8_t m3_tileHigh;
			uint16_t m3_spriteToPush;
			SpriteFetcherStep m3_spriteStep;
			int m3_spriteXOffset;
			int m3_spriteYOffset;
			uint16_t m3_spriteAddress;
			uint8_t m3_spriteControl;
			uint8_t m3_spriteLow;
			uint8_t m3_spriteHigh;

			// Mode 1
			bool m_switchToHBlank;
	};
}

#endif
