#include "graphics/LCDController.hpp"

#define COLORVALUE_BLANK 0xFF
#define BACKGROUND_PALETTE 0xFF
#define BACKGROUND_INDEX 0xFFFF
#define COLOR_BLANK 0x7FFF;
#define clearQueue(q) while (!q.empty()) q.pop()

namespace toygb {
	const uint8_t OBJECT_HEIGHTS[] = {8, 16};
	const uint16_t DMG_PALETTE[] = {0x6318, 0x4A52, 0x2108, 0x18C6};

	LCDController::LCDController(){
		m_vram = nullptr;
		m_oam = nullptr;

		m_hdma = nullptr;
		m_cgbPalette = nullptr;
		m_lcdControl = nullptr;
		m_vramBankMapping = nullptr;
		m_vramMapping = nullptr;
		m_oamMapping = nullptr;

		m_backBuffer = nullptr;
	}

	LCDController::~LCDController(){
		if (m_vram != nullptr) delete[] m_vram;
		if (m_oam != nullptr) delete[] m_oam;

		if (m_hdma != nullptr) delete m_hdma;
		if (m_cgbPalette != nullptr) delete m_cgbPalette;
		if (m_lcdControl != nullptr) delete m_lcdControl;
		if (m_vramBankMapping != nullptr) delete m_vramBankMapping;
		if (m_vramMapping != nullptr) delete m_vramMapping;
		if (m_oamMapping != nullptr) delete m_oamMapping;

		if (m_backBuffer != nullptr) delete m_backBuffer;

		m_vram = nullptr;
		m_oam = nullptr;

		m_hdma = nullptr;
		m_cgbPalette = nullptr;
		m_lcdControl = nullptr;
		m_vramBankMapping = nullptr;
		m_vramMapping = nullptr;
		m_oamMapping = nullptr;
	}

	void LCDController::init(OperationMode mode, InterruptVector* interrupt){
		m_mode = mode;
		m_interrupt = interrupt;

		m_oam = new uint8_t[OAM_SIZE];
		for (int i = 0; i < OAM_SIZE; i++) m_oam[i] = 0;

		if (mode == OperationMode::DMG) {
			m_vram = new uint8_t[VRAM_SIZE];
			for (int i = 0; i < VRAM_SIZE; i++) m_vram[i] = 0;
		} else if (mode == OperationMode::CGB){
			m_vram = new uint8_t[VRAM_BANK_SIZE * VRAM_BANK_NUM];
			for (int i = 0; i < VRAM_BANK_SIZE * VRAM_BANK_NUM; i++) m_vram[i] = 0;
		}
		m_vramBank = 0;
	}

	void LCDController::configureMemory(MemoryMap* memory){
		m_oamMapping = new LCDMemoryMapping(m_oam);
		m_lcdControl = new LCDControlMapping();

		memory->add(OAM_OFFSET, OAM_END_OFFSET - 1, m_oamMapping);
		memory->add(IO_LCD_CONTROL, IO_WINDOW_X, m_lcdControl);

		if (m_mode == OperationMode::DMG){
			m_vramMapping = new LCDMemoryMapping(m_vram);
		} else if (m_mode == OperationMode::CGB){
			m_hdma = new HDMAMapping();
			m_cgbPalette = new CGBPaletteMapping();
			m_vramBankMapping = new VRAMBankSelectMapping(&m_vramBank);
			m_vramMapping = new LCDBankedMemoryMapping(&m_vramBank, VRAM_BANK_SIZE, m_vram);

			memory->add(IO_HDMA_SOURCELOW, IO_HDMA_SETTINGS, m_hdma);
			memory->add(IO_BGPALETTE_INDEX, IO_OBJPALETTE_DATA, m_cgbPalette);
			memory->add(IO_VRAM_BANK, IO_VRAM_BANK, m_vramBankMapping);
		}
		memory->add(VRAM_OFFSET, VRAM_OFFSET + VRAM_SIZE - 1, m_vramMapping);
	}

#define dot(num) co_await std::suspend_always()

	GBComponent LCDController::run(){
		m_frontBuffer = new uint16_t[LCD_WIDTH * LCD_HEIGHT];
		m_backBuffer = new uint16_t[LCD_WIDTH * LCD_HEIGHT];
		std::deque<uint16_t> selectedSprites;

		LCDController::ObjectSelectionComparator objComparator(m_mode, m_oamMapping);
		while (true){
			if (m_lcdControl->displayEnable){
				uint16_t* tmp = m_frontBuffer;
				m_frontBuffer = m_backBuffer;
				m_backBuffer = tmp;

				for (int line = 0; line < 144; line++){
					selectedSprites.clear();

					// Mode 2 = OAM scan
					m_lcdControl->modeFlag = 2;
					m_oamMapping->accessible = false;
					m_vramMapping->accessible = true;
					if (m_cgbPalette != nullptr) m_cgbPalette->accessible = true;

					m_lcdControl->coordY = line;
					m_lcdControl->coincidenceFlag = (m_lcdControl->coordY == m_lcdControl->coordYCompare);
					if (m_lcdControl->coincidenceFlag && m_lcdControl->lycInterrupt){
						m_interrupt->setRequest(Interrupt::LCDStat);
					}

					if (m_lcdControl->oamInterrupt){
						m_interrupt->setRequest(Interrupt::LCDStat);
					}

					for (int obj = 0; obj < 40; obj++){
						uint16_t oamAddress = 4 * obj;
						uint8_t yposition = m_oamMapping->lcdGet(oamAddress) - 16;
						if (yposition <= line && line < yposition + OBJECT_HEIGHTS[m_lcdControl->objectSize] && selectedSprites.size() < 10){
							selectedSprites.push_back(oamAddress);
						}
						dot(); dot();
					}
					std::sort(selectedSprites.begin(), selectedSprites.end(), objComparator);

					//if (selectedSprites.size() > 10)
					//	selectedSprites.erase(selectedSprites.begin() + 10, selectedSprites.end());

					// Mode 3 = Drawing pixels
					m_lcdControl->modeFlag = 3;
					m_oamMapping->accessible = false;
					m_vramMapping->accessible = false;
					if (m_cgbPalette != nullptr) m_cgbPalette->accessible = false;

					std::queue<LCDController::Pixel> backgroundQueue;
					std::queue<LCDController::Pixel> objectQueue;

					bool wasInsideWindow = false;
					int hblankDuration = 204;
					for (int x = 0; x < LCD_WIDTH; x++){
						bool insideWindow = (x >= m_lcdControl->windowX && line >= m_lcdControl->windowY);
						if (insideWindow != wasInsideWindow)
							clearQueue(backgroundQueue);

						// Fetch pixels
						if (backgroundQueue.empty()){
							uint16_t tileMapAddress = insideWindow ? (m_lcdControl->windowDisplaySelect ? 0x1C00 : 0x1800) : (m_lcdControl->backgroundDisplaySelect ? 0x1C00 : 0x1800);

							uint8_t tileX, tileY, indexY;
							if (insideWindow){  // FIXME
								tileX = ((x - m_lcdControl->windowX) >> 3) & 0x1F;
								tileY = ((line - m_lcdControl->windowY) >> 3) & 0x1F;
								indexY = (line - m_lcdControl->windowY) & 7;
							} else {  // FIXME
								tileX = ((m_lcdControl->scrollX + x) >> 3) & 0x1F;
								tileY = ((m_lcdControl->scrollY + line) >> 3) & 0x1F;
								indexY = (m_lcdControl->scrollY + 1) & 7;
							}

							uint8_t tileIndex = m_vram[tileMapAddress + 32 * tileY + tileX];  // ?
							dot(); dot();

							uint16_t tileAddress;
							if (m_lcdControl->backgroundDataSelect){
								tileAddress = 0x0000 + tileIndex * 16;
							} else if (tileIndex >= 128) {
								tileAddress = 0x0800 + (tileIndex - 128) * 16;
							} else {
								tileAddress = 0x1000 + tileIndex * 16;
							}
							uint8_t tileLow = m_vramMapping->lcdGet(tileAddress + indexY * 2);
							uint8_t tileHigh = m_vramMapping->lcdGet(tileAddress + indexY * 2 + 1);
							dot(); dot(); dot(); dot(); dot(); dot();

							for (int i = 7; i >= 0; i--){
								uint8_t color = (((tileHigh >> i) & 1) << 1) | ((tileLow >> i) & 1);
								LCDController::Pixel pixelData(color, BACKGROUND_PALETTE, BACKGROUND_INDEX, false);
								backgroundQueue.push(pixelData);
							}
							dot();
						}

						// Fetch sprites
						if (m_lcdControl->objectEnable){
							bool pushSprite = objectQueue.empty() || (m_mode == OperationMode::CGB && selectedSprites.front() < objectQueue.front().oamAddress);
							if (pushSprite){
								clearQueue(objectQueue);
								uint16_t spriteToPush;

								while (!selectedSprites.empty() && m_oamMapping->lcdGet(selectedSprites.front() + 1) - 8 < x - 8) {
									//std::cout << "pop " << oh16(selectedSprites.front()) << " at (" << x << ", " << line << "), OAM " << "(" << m_oamMapping->lcdGet(selectedSprites.front() + 1) - 8 << ", " << m_oamMapping->lcdGet(selectedSprites.front()) - 16 << ")"  << std::endl;
									selectedSprites.pop_front();
								}

								if (!selectedSprites.empty() && x >= m_oamMapping->lcdGet(selectedSprites.front() + 1) - 8 && x < m_oamMapping->lcdGet(selectedSprites.front() + 1) - 8 + 8){
									if (m_mode == OperationMode::DMG){
										spriteToPush = selectedSprites.front();
										//std::cout << "pop " << oh16(selectedSprites.front()) << " at (" << x << ", " << line << "), OAM " << "(" << m_oamMapping->lcdGet(selectedSprites.front() + 1) - 8 << ", " << m_oamMapping->lcdGet(selectedSprites.front()) - 16 << ")"  << std::endl;
										selectedSprites.pop_front();
									} else if (m_mode == OperationMode::CGB) {
										spriteToPush = selectedSprites.front();
										for (std::deque<uint16_t>::iterator it = selectedSprites.begin() + 1; it != selectedSprites.end() && m_oamMapping->lcdGet(*it + 1) - 8 < x + 8; it++){
											if (*it < spriteToPush)
												spriteToPush = *it;
										}
									}
									dot(); hblankDuration -= 1;

									uint8_t scrollOffset = m_lcdControl->scrollX % 8;
									if (scrollOffset > 0 && x == 0){
										for (int i = 0; i < scrollOffset + 4; i++) dot();
										hblankDuration -= (scrollOffset + 4);
									}

									uint8_t tileIndex = m_oamMapping->lcdGet(spriteToPush + 2);
									int xoffset = x - (m_oamMapping->lcdGet(spriteToPush + 1) - 8);
									int yoffset = line - (m_oamMapping->lcdGet(spriteToPush) - 16);
									uint8_t control = m_oamMapping->lcdGet(spriteToPush + 3);

									uint16_t tileAddress = tileIndex * 16;
									if (m_lcdControl->objectSize)
										tileAddress &= 0xFFFE;

									uint8_t tileLow = m_vramMapping->lcdGet(tileAddress + 2*yoffset);
									uint8_t tileHigh = m_vramMapping->lcdGet(tileAddress + 2*yoffset + 1);
									//std::cout << oh16(tileAddress) << " " << xoffset << " " << yoffset << " " << oh8(tileLow) << " " << oh8(tileHigh);
									dot(); hblankDuration -= 1;

									for (int i = 7 - xoffset; i >= 0; i--){
										uint8_t color = (((tileHigh >> i) & 1) << 1) | ((tileLow >> i) & 1);
										uint8_t palette;
										if (m_mode == OperationMode::CGB){
											palette = control & 7;
										} else if (m_mode == OperationMode::DMG){
											palette = (control >> 4) & 1;
										}
										LCDController::Pixel pixel(color, palette, spriteToPush, (control >> 7) & 1);
										objectQueue.push(pixel);
									}
								}
							}
						}

						// Pixel rendering
						LCDController::Pixel backgroundPixel = backgroundQueue.front();
						backgroundQueue.pop();

						uint8_t colorValue;
						if (m_mode == OperationMode::DMG && !m_lcdControl->backgroundDisplay){
							colorValue = COLORVALUE_BLANK;
						} else {
							colorValue = backgroundPixel.color;
						}
						uint8_t colorPalette = backgroundPixel.palette;
						uint16_t elementIndex = backgroundPixel.oamAddress;

						if (!objectQueue.empty()){
							LCDController::Pixel objectPixel = objectQueue.front();
							objectQueue.pop();

							bool objectHasPriority;
							if (objectPixel.priority){  // Flag BG over OBJ
								if (!m_lcdControl->backgroundDisplay){
									objectHasPriority = (objectPixel.color > 0);
								} else {
									objectHasPriority = (backgroundPixel.color == 0);
								}
							} else {
								objectHasPriority = (objectPixel.color > 0);
							}

							if (objectHasPriority){
								colorValue = objectPixel.color;
								colorPalette = objectPixel.palette;
								elementIndex = objectPixel.oamAddress;
							}
						}

						uint16_t colorResult;
						if (colorValue == COLORVALUE_BLANK){
							colorResult = COLOR_BLANK;
						} else if (m_mode == OperationMode::DMG){
							if (colorPalette == BACKGROUND_PALETTE){
								colorResult = DMG_PALETTE[m_lcdControl->backgroundPalette[colorValue]];
							} else if (colorPalette == 0){
								colorResult = DMG_PALETTE[m_lcdControl->objectPalette0[colorValue]];
							} else if (colorPalette == 1){
								colorResult = DMG_PALETTE[m_lcdControl->objectPalette1[colorValue]];
							}
						} else if (m_mode == OperationMode::CGB) {
							int paletteIndex = colorPalette * 4 + colorValue;
							if (elementIndex == BACKGROUND_INDEX){
								colorResult = m_cgbPalette->backgroundPalettes[paletteIndex];
							} else {
								colorResult = m_cgbPalette->objectPalettes[paletteIndex];
							}
						}

						m_backBuffer[line * LCD_WIDTH + x] = colorResult;

						wasInsideWindow = insideWindow;
					}

					// Mode 0 = HBlank
					m_lcdControl->modeFlag = 0;
					m_oamMapping->accessible = true;
					m_vramMapping->accessible = true;
					if (m_cgbPalette != nullptr) m_cgbPalette->accessible = true;

					if (m_lcdControl->hblankInterrupt){
						m_interrupt->setRequest(Interrupt::LCDStat);
					}

					for (int i = 0; i < hblankDuration; i++){
						dot();  // TODO
					}
				}
				// Mode 1 = VBlank
				m_lcdControl->modeFlag = 1;
				m_oamMapping->accessible = true;
				m_vramMapping->accessible = true;
				if (m_cgbPalette != nullptr) m_cgbPalette->accessible = true;

				m_interrupt->setRequest(Interrupt::VBlank);

				for (int line = 144; line < 153; line++){
					m_lcdControl->coordY = line;
					m_lcdControl->coincidenceFlag = (m_lcdControl->coordY == m_lcdControl->coordYCompare);

					for (int i = 0; i < 456; i++){
						dot();  // TODO
					}
				}
			} else {
				dot();
			}
		}
	}

	uint16_t* LCDController::pixels(){
		return m_frontBuffer;
	}


	// LCDController::ObjectSelectionComparator

	LCDController::ObjectSelectionComparator::ObjectSelectionComparator(OperationMode mode, LCDMemoryMapping* oam){
		m_mode = mode;
		m_oamMapping = oam;
	}

	bool LCDController::ObjectSelectionComparator::operator()(const uint16_t& address1, const uint16_t& address2){
		return m_oamMapping->lcdGet(address1 + 1) < m_oamMapping->lcdGet(address2 + 1);
	}


	// LCDController::Pixel

	LCDController::Pixel::Pixel(uint8_t pcolor, uint8_t ppalette, uint16_t paddress, bool ppriority){
		color = pcolor;
		palette = ppalette;
		oamAddress = paddress;
		priority = ppriority;
	}
}
