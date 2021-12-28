#include "graphics/LCDController.hpp"

// Special color value for a blank pixel (nothing at all, theoretically is is whiter than drawn white pixels)
#define COLORVALUE_BLANK 0xFF

// Special value for the DMG background index
#define BACKGROUND_PALETTE 0xFF

// Special value for Pixel::oamAddress to tell it is a background pixel
#define BACKGROUND_INDEX 0xFFFF

// CGB RGB color value for a blank pixel
#define COLOR_BLANK 0x6318

namespace toygb {
	// Possible objects heights, defined by LCDC.2, in pixels
	const uint8_t OBJECT_HEIGHTS[] = {8, 16};

	// Default RGB555 color values for DMG colors
	const uint16_t DMG_PALETTE[] = {0x6318, 0x4A52, 0x2108, 0x18C6};

	// Possible tile map positions for the window and background (index is defined by LCDC.3 (background) and LCDC.6 (window))
	const uint16_t TILEMAP_VRAM_ADDRESS[] = {0x1800, 0x1C00};

	/** FIXME : This is not a fully cycle-accurate implementation of the PPU, many details remain inaccurate or unclear.
	 *  As of now it works well enough to emulate most games relatively well, and passes Matt Curie's dmg_acid2 test
	 *  So TODO : Make this better */

	// Initialize the component with null values (the actual initialization is in LCDController::init)
	LCDController::LCDController() {
		m_vram = nullptr;
		m_oam = nullptr;

		m_hdma = nullptr;
		m_dmgPalette = nullptr;
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
		if (m_dmgPalette != nullptr) delete m_dmgPalette;
		if (m_cgbPalette != nullptr) delete m_cgbPalette;
		if (m_lcdControl != nullptr) delete m_lcdControl;
		if (m_vramBankMapping != nullptr) delete m_vramBankMapping;
		if (m_vramMapping != nullptr) delete m_vramMapping;
		if (m_oamMapping != nullptr) delete m_oamMapping;

		if (m_backBuffer != nullptr) delete m_backBuffer;
		if (m_frontBuffer != nullptr) delete m_frontBuffer;

		m_vram = m_oam = nullptr;

		m_hdma = nullptr;
		m_dmgPalette = nullptr;
		m_cgbPalette = nullptr;
		m_lcdControl = nullptr;
		m_vramBankMapping = nullptr;
		m_vramMapping = nullptr;
		m_oamMapping = nullptr;

		m_backBuffer = m_frontBuffer = nullptr;
	}

	// Initialize the component
	void LCDController::init(HardwareConfig& hardware, InterruptVector* interrupt) {
		m_hardware = hardware;
		m_interrupt = interrupt;

		m_oam = new uint8_t[OAM_SIZE];
		for (int i = 0; i < OAM_SIZE; i++)  // FIXME : Clear OAM at boot ?
			m_oam[i] = 0;

		switch (hardware.mode()) {
			case OperationMode::DMG:
				m_vram = new uint8_t[VRAM_SIZE];
				for (int i = 0; i < VRAM_SIZE; i++)  // FIXME : Clear VRAM at boot ?
					m_vram[i] = 0;
				break;
			case OperationMode::CGB:
				m_vram = new uint8_t[VRAM_BANK_SIZE * VRAM_BANK_NUM];
				for (int i = 0; i < VRAM_BANK_SIZE * VRAM_BANK_NUM; i++)  // FIXME : Clear VRAM at boot ?
					m_vram[i] = 0;
				break;
			case OperationMode::Auto:
				throw EmulationError("OperationMode::Auto given to LCD controller");
		}
		m_vramBank = 0;

		m_oamMapping = new OAMMapping(hardware, m_oam);
		m_lcdControl = new LCDControlMapping();
		m_dmgPalette = new DMGPaletteMapping();
	}

	// Configure the associated memory mappings
	void LCDController::configureMemory(MemoryMap* memory) {
		memory->add(OAM_OFFSET, IO_OFFSET - 1, m_oamMapping);  // Map the OAM mapping on 0xFE00-0xFEFF as it also handles the "forbidden" 0xFEA0-0xFEFF area
		memory->add(IO_LCD_CONTROL, IO_COORD_COMPARE, m_lcdControl);
		memory->add(IO_BG_PALETTE, IO_WINDOW_X, m_dmgPalette);

		switch (m_hardware.mode()) {
			case OperationMode::DMG:
				m_vramMapping = new LCDMemoryMapping(m_vram);
				break;

			case OperationMode::CGB:
				m_hdma = new HDMAMapping();
				m_cgbPalette = new CGBPaletteMapping();
				m_vramBankMapping = new VRAMBankSelectMapping(&m_vramBank);
				m_vramMapping = new LCDBankedMemoryMapping(&m_vramBank, VRAM_BANK_SIZE, m_vram);

				memory->add(IO_HDMA_SOURCELOW, IO_HDMA_SETTINGS, m_hdma);
				memory->add(IO_BGPALETTE_INDEX, IO_OBJPALETTE_DATA, m_cgbPalette);
				memory->add(IO_VRAM_BANK, IO_VRAM_BANK, m_vramBankMapping);
				break;

			case OperationMode::Auto:
				throw EmulationError("OperationMode::Auto given to LCD controller");
		}
		memory->add(VRAM_OFFSET, VRAM_OFFSET + VRAM_SIZE - 1, m_vramMapping);
	}

// Wait till the next clock in the coroutine
#define dot() co_await std::suspend_always()

	// Main coroutine component. This could be better if it was split into smaller functions, but the coroutine management forces it to be in one block
	GBComponent LCDController::run() {
		// Allocate the pixel buffers
		m_frontBuffer = new uint16_t[LCD_WIDTH * LCD_HEIGHT];
		m_backBuffer = new uint16_t[LCD_WIDTH * LCD_HEIGHT];
		std::deque<uint16_t> selectedSprites;  // Will contain the objects selected for the current scanline
		LCDController::ObjectSelectionComparator objComparator(m_hardware, m_oamMapping);

		while (true) {
			if (m_lcdControl->displayEnable) {  // FIXME : shutting down the display should stop it right away, not at the next frame
				// Swap the pixel buffers
				uint16_t* tmp = m_frontBuffer;
				m_frontBuffer = m_backBuffer;
				m_backBuffer = tmp;

				// Window rendering does not uses the position of the screen, but instead counts the lines already rendered during the current frame
				// This is important if the window is enabled then moved within a frame
				int windowLineCounter = 0;

				// On-screen scanlines : 0-143
				for (int line = 0; line < 144; line++) {
					selectedSprites.clear();
					bool hasWindow = false;  // Tell whether the window was rendered on this scanline, to increment the window line counter

					// Mode 2 : OAM scan
					// Find all sprites that will be rendered on the current scanline

					m_lcdControl->modeFlag = 2;
					m_oamMapping->accessible = false;
					m_vramMapping->accessible = true;
					if (m_cgbPalette != nullptr)
						m_cgbPalette->accessible = true;

					// Update LY and check LY = LYC STAT interrupt
					m_lcdControl->coordY = line;
					if (m_lcdControl->coordY == m_lcdControl->coordYCompare && m_lcdControl->lycInterrupt) {
						m_interrupt->setRequest(Interrupt::LCDStat);
					}

					if (m_lcdControl->oamInterrupt) {
						m_interrupt->setRequest(Interrupt::LCDStat);
					}

					// Scan all objects in OAM
					// Small inaccuracy : actually, OAM is scanned row by row, with a row = 8 bytes = 2 objects. FIXME : Does this have any incidence ?
					// NOTE : the position described in OAM is (X + 8, Y + 16), to allow hiding a sprite by setting its coordinates to 0
					// Sprites hidden by their X coordinate (X = 0 or X >= 160) still count on their scanlines
					for (int obj = 0; obj < 40; obj++) {
						uint16_t oamAddress = 4 * obj;
						uint8_t yposition = m_oamMapping->lcdGet(oamAddress) - 16;
						// yposition <= line < yposition + object height : this sprite is on the current scanline
						// Only 10 sprites can be rendered on any given scanline, objects hidden by their X coordinate (X = 0 or X >= 160) still count for this limit
						if (yposition <= line && line < yposition + OBJECT_HEIGHTS[m_lcdControl->objectSize] && selectedSprites.size() < 10)
							selectedSprites.push_back(oamAddress);

						dot(); dot();
					}
					std::sort(selectedSprites.begin(), selectedSprites.end(), objComparator);

					// Mode 3 = Drawing pixels
					m_lcdControl->modeFlag = 3;
					m_oamMapping->accessible = false;
					m_vramMapping->accessible = false;
					if (m_cgbPalette != nullptr)
						m_cgbPalette->accessible = false;

					// Pixel FIFOs
					std::deque<LCDController::Pixel> backgroundQueue;
					std::deque<LCDController::Pixel> objectQueue;

					bool wasInsideWindow = false;  // Keep track of what we were rendering (background / window) to clear the background FIFO accordingly
					int hblankDuration = 204;      // Most sprite operations are in addition to the basic mode 3 duration, so shorten HBlank
					for (int x = 0; x < LCD_WIDTH; x++) {
						// The WX register is window X coordinate + 7
						bool insideWindow = m_lcdControl->windowEnable && (x >= m_dmgPalette->windowX - 7 && line >= m_dmgPalette->windowY);
						if (insideWindow != wasInsideWindow)  // Clear the background queue if we were rendering the background and switch to the window (and vice-versa)
							backgroundQueue.clear();
						hasWindow |= insideWindow;

						////////// Fetch background pixels

						// Only reload the background FIFO once it is empty. The tile fetch operation here is wildly inaccurate.
						if (backgroundQueue.empty()) {
							// Get the tile index from the selected tilemap
							uint16_t tileMapAddress = TILEMAP_VRAM_ADDRESS[insideWindow ? m_lcdControl->windowTilemapSelect : m_lcdControl->backgroundTilemapSelect];
							uint8_t tileX, tileY, indexY;
							if (insideWindow) {  // Window : scrolling registers define the position of the window within the screen -> x - WX
								tileX = ((x - (m_dmgPalette->windowX - 7)) >> 3) & 0x1F;  // X tile index in the tilemap (>> 3 because tiles are 8x8 pixels)
								tileY = (windowLineCounter >> 3) & 0x1F;                  // Y tile index in the tilemap
								indexY = windowLineCounter & 7;                           // Y offset of the current scanline / window line relative to the tile
							} else {  // Background : scrolling registers define the position of the screen within the background -> x + SCX
								tileX = ((m_lcdControl->scrollX + x) >> 3) & 0x1F;
								tileY = ((m_lcdControl->scrollY + line) >> 3) & 0x1F;
								indexY = (m_lcdControl->scrollY + line) & 7;
							}

							// The tilemap is a 32*32 array, most significant coordinate is the row
							uint8_t tileIndex = m_vram[tileMapAddress + 32 * tileY + tileX];
							dot(); dot();

							// Get the tile data VRAM address from its index (all addresses are relative to their memory section, here relative to VRAM)
							// Tiles 128-255 are always in 0x0800-0x0FFF, regardless of the tile data selector
							// Then LCDC.4 = 1 -> tiles 0-127 are in 0x0000-0x07FF, LCDC.4 = 0 -> tiles 0-127 are in 0x1000-0x17FF
							// Each tile data is 16 bytes, 8 rows of 2 bytes
							uint16_t tileAddress;
							if (tileIndex >= 128)
								tileAddress = 0x0800 + (tileIndex - 128) * 16;
							else if (m_lcdControl->backgroundDataSelect)  // LCDC.4 = 1 : Address from 0x0000
								tileAddress = 0x0000 + tileIndex * 16;
							else  // LCDC.4 = 0 : Address from 0x1000
								tileAddress = 0x1000 + tileIndex * 16;

							// Retrieve the 2-byte row to render from VRAM
							uint8_t tileLow = m_vramMapping->lcdGet(tileAddress + indexY * 2);
							uint8_t tileHigh = m_vramMapping->lcdGet(tileAddress + indexY * 2 + 1);  // (Little endian, upper byte is second)
							dot(); dot(); dot(); dot(); dot(); dot();

							// Each row is in two bytes, bits of each byte are interleaved to build the 2-bits color index. Indices are the x coordinate within the tile :
							// Upper byte : u0 u1 u2 u3 u4 u5 u6 u7 |
							// Lower byte : l0 l1 l2 l3 l4 l5 l6 l7 | -> u0l0 u1l1 u2l2 u3l3 u4l4 u5l5 u6l6 u7l7
							for (int i = 7; i >= 0; i--) {
								uint8_t color = (((tileHigh >> i) & 1) << 1) | ((tileLow >> i) & 1);
								LCDController::Pixel pixelData(color, BACKGROUND_PALETTE, BACKGROUND_INDEX, false);
								backgroundQueue.push_back(pixelData);
							}
							dot();
						}

						////////// Fetch sprites
						// NOTE : selectedSprites contains the OAM addresses of the selected sprites, but is sorted by sprite X coordinate
						if (m_lcdControl->objectEnable) {
							// Check whether a new sprite may be pushed
							// In DMG mode, lower X coordinate gets priority, so a sprite already being drawn can not be overridden by a later one
							// In CGB mode, lower OAM position gets priority, so a sprite already being drawn can be overridden if a next one has a lower oam address
							// FIXME : Here we only check the immediately next one. If several sprites are in range, can a sprite 2 positions later preempt the current one ?
							bool pushSprite = objectQueue.empty() || (m_hardware.mode() == OperationMode::CGB && selectedSprites.front() < objectQueue.front().oamAddress);
							if (pushSprite) {
								objectQueue.clear();
								uint16_t spriteToPush;

								// Eliminate sprites that were earlier on the line (the current X coordinate is after their last pixel X coordinate)
								// OAM defines the X coordinate + 8, so to detect this it's OAM X coordinate - 8 < current X - 8 <=> OAM X coordinate < current X
								while (!selectedSprites.empty() && m_oamMapping->lcdGet(selectedSprites.front() + 1) < x)
									selectedSprites.pop_front();

								// Check whether the next sprite must be rendered at the current X coordinate (only need to check the next one as selectedSprites is sorted by X coordinate).
								// As always, OAM gives X + 8, so -8 everywhere to get the actual position on the screen (and the end position is OAM X coordinate - 8 + 8 = OAM X coordinate)
								if (!selectedSprites.empty() && x >= m_oamMapping->lcdGet(selectedSprites.front() + 1) - 8 && x < m_oamMapping->lcdGet(selectedSprites.front() + 1)) {
									switch (m_hardware.mode()) {
										case OperationMode::DMG:  // DMG mode : No problem, priority goes to the lowest X coordinate, so the first in selectedSprites
											spriteToPush = selectedSprites.front();
											selectedSprites.pop_front();
											break;
										case OperationMode::CGB:  // CGB mode : Priority goes to the lowest OAM address, so we need to scan sprites in the [x, x+8] range to find the one to render
											spriteToPush = selectedSprites.front();
											for (std::deque<uint16_t>::iterator it = selectedSprites.begin() + 1; it != selectedSprites.end() && m_oamMapping->lcdGet(*it + 1) - 8 < x + 8; it++) {
												if (*it < spriteToPush)
													spriteToPush = *it;
											}
											break;
										case OperationMode::Auto:
											throw EmulationError("OperationMode::Auto given to LCD controller");
									}
									dot(); hblankDuration -= 1;

									// FIXME : Delay if the background scrolling is not a multiple of 8. This is probably not accurate at all.
									uint8_t scrollOffset = m_lcdControl->scrollX % 8;
									if (scrollOffset > 0 && x == 0) {
										for (int i = 0; i < scrollOffset + 4; i++)
											dot();
										hblankDuration -= (scrollOffset + 4);
									}

									// Read the OAM entry of the sprite to render
									// FIXME : Isn't this done in OAM scan ?
									uint8_t tileIndex = m_oamMapping->lcdGet(spriteToPush + 2);
									int xoffset = x - (m_oamMapping->lcdGet(spriteToPush + 1) - 8);
									int yoffset = line - (m_oamMapping->lcdGet(spriteToPush) - 16);
									uint8_t control = m_oamMapping->lcdGet(spriteToPush + 3);

									// If sprites are 8x16 (= 2 tiles), tile index alignment to a multiple of 2 is enforced by the PPU by always clearing the lowest bit
									if (m_lcdControl->objectSize)
										tileIndex &= 0xFE;

									// Contrary to the background / window, sprites always use the 0x0000-0x1000 tile data addressing, so no problem at all
									uint16_t tileAddress = tileIndex * 16;

									// If OAM control byte, bit 6 is set, the tile is flipped vertically
									// We thus count the tile data rows from the end instead of from the beginning (row 2 -> row 7-2 / 15-2)
									if ((control >> 6) & 1)
										yoffset = OBJECT_HEIGHTS[m_lcdControl->objectSize] - yoffset - 1;

									// Retrieve the tile data row from VRAM, much like background
									uint8_t tileLow = m_vramMapping->lcdGet(tileAddress + 2*yoffset);
									uint8_t tileHigh = m_vramMapping->lcdGet(tileAddress + 2*yoffset + 1);
									dot(); hblankDuration -= 1;

									// See above, bits organisation and color bits interleave are described in the same part of the background row fetching
									for (int i = 7 - xoffset; i >= 0; i--) {
										uint8_t color;

										// Objects can be flipped horizontally with bit 5 of the OAM control byte
										// We do that by taking bits from the end instead of from the beginning, much like Y flip
										if ((control >> 5) & 1)  // Flipped horizontally
											color = (((tileHigh >> (7 - i)) & 1) << 1) | ((tileLow >> (7 - i)) & 1);
										else
											color = (((tileHigh >> i) & 1) << 1) | ((tileLow >> i) & 1);

										// Get which palette to use from the OAM control byte
										uint8_t palette;
										switch (m_hardware.mode()) {
											case OperationMode::DMG:  // DMG mode : set by bit 4 of the control byte (0 = OBP0, 1 = OBP1)
												palette = (control >> 4) & 1;
												break;
											case OperationMode::CGB:  // CGB mode : OBPS/OBPD palette index is defined by bits 0-3
												palette = control & 7;
												break;
											case OperationMode::Auto:
												throw EmulationError("OperationMode::Auto given to LCD controller");
										}

										// Object-to-background priority is set by bit 7 of the control byte (0 = object colors 1-3 above background, 1 = background colors 1-3 above objects)
										LCDController::Pixel pixel(color, palette, spriteToPush, (control >> 7) & 1);
										objectQueue.push_back(pixel);
									}
								}
							}
						}


						////////// Pixel rendering
						// NOTE : Here, "colorValue" means the color index before resolving it with a palette, "color" means the final color that will get displayed, after palettes resolution

						// If the background scrolling is not a multiple of 8 (= on the exact edge of a tile),
						// eliminate the pixels of the first tile that are outside of the screen on the left
						if (x == 0) {
							for (int i = 0; i < m_lcdControl->scrollX % 8; i++)
								backgroundQueue.pop_front();
						}
						LCDController::Pixel backgroundPixel = backgroundQueue.front();
						backgroundQueue.pop_front();

						// In DMG mode, if the LCDC.0 is clear, background is disabled so everything not covered by sprites is blank
						// In CGB mode, this does only affect the background-to-object priority, that is handled later on
						uint8_t colorValue;
						if (m_hardware.mode() == OperationMode::DMG && !m_lcdControl->backgroundDisplay)
							colorValue = COLORVALUE_BLANK;
						else
							colorValue = backgroundPixel.color;

						uint8_t colorPalette = backgroundPixel.palette;
						uint16_t elementIndex = backgroundPixel.oamAddress;

						// If there is an object pixel AND a background pixel, select which one to render
						if (!objectQueue.empty()) {
							LCDController::Pixel objectPixel = objectQueue.front();
							objectQueue.pop_front();

							// TODO : Add CGB-mode priority shenanigans
							// Basically, in order of decreasing importance [CGB LCDC.0 > BG priority bit >] Object priority bit
							// The logic is heavily reduced, but here is a table to explain all that (- = any value) :
							// Operation | LCDC.0 (background | BG priority bit (VRAM  | Object priority bit   | Object value | Background value | Value to
							// mode      | enable / priority) | bank 1 attribute byte) | (in OAM control byte) |              |                  | render
							// --------- | ------------------ | ---------------------- | --------------------- | ------------ | ---------------- | --------
							//       DMG |                  1 |                        |                     0 |         zero |             zero | BG
							//       DMG |                  1 |                        |                     0 |     non-zero |             zero | OBJ
							//       DMG |                  1 |                        |                     0 |         zero |         non-zero | BG
							//       DMG |                  1 |                        |                     0 |     non-zero |         non-zero | OBJ
							//       DMG |                  1 |                        |                     1 |         zero |             zero | BG
							//       DMG |                  1 |                        |                     1 |     non-zero |             zero | OBJ
							//       DMG |                  1 |                        |                     1 |         zero |         non-zero | BG
							//       DMG |                  1 |                        |                     1 |     non-zero |         non-zero | BG
							//       DMG |                  0 |                        |                     - |         zero |                - | blank
							//       DMG |                  0 |                        |                     - |     non-zero |                - | OBJ
							// --------- | ------------------ | ---------------------- | --------------------- | ------------ | ---------------- | --------
							//       CGB |                  1 |                      0 |                     0 |         zero |             zero | BG
							//       CGB |                  1 |                      0 |                     0 |     non-zero |             zero | OBJ
							//       CGB |                  1 |                      0 |                     0 |         zero |         non-zero | BG
							//       CGB |                  1 |                      0 |                     0 |     non-zero |         non-zero | OBJ
							//       CGB |                  1 |                      0 |                     1 |         zero |             zero | BG
							//       CGB |                  1 |                      0 |                     1 |     non-zero |             zero | OBJ
							//       CGB |                  1 |                      0 |                     1 |         zero |         non-zero | BG
							//       CGB |                  1 |                      0 |                     1 |     non-zero |         non-zero | BG
							//       CGB |                  1 |                      1 |                     - |         zero |             zero | BG
							//       CGB |                  1 |                      1 |                     - |     non-zero |             zero | OBJ
							//       CGB |                  1 |                      1 |                     - |         zero |         non-zero | BG
							//       CGB |                  1 |                      1 |                     - |     non-zero |         non-zero | BG
							//       CGB |                  0 |                      - |                     - |         zero |             zero | BG
							//       CGB |                  0 |                      - |                     - |     non-zero |             zero | OBJ
							//       CGB |                  0 |                      - |                     - |         zero |         non-zero | BG
							//       CGB |                  0 |                      - |                     - |     non-zero |         non-zero | OBJ
							bool objectHasPriority;
							if (objectPixel.priority) {
								if (!m_lcdControl->backgroundDisplay) {
									objectHasPriority = (objectPixel.color > 0);
								} else {
									objectHasPriority = (backgroundPixel.color == 0);
								}
							} else {
								objectHasPriority = (objectPixel.color > 0);
							}

							// If we determined the object pixel must replace the background one
							if (objectHasPriority) {
								colorValue = objectPixel.color;
								colorPalette = objectPixel.palette;
								elementIndex = objectPixel.oamAddress;
							}
						}

						// Resolve the actual color to render with the color index and the palette
						uint16_t colorResult;

						if (colorValue == COLORVALUE_BLANK) {
							colorResult = COLOR_BLANK;
						}

						// DMG mode : Resolve with monochrome palettes (BGP / OBP0 / OBP1) and use the associated default RGB555 color
						else if (m_hardware.mode() == OperationMode::DMG) {
							if (colorPalette == BACKGROUND_PALETTE)
								colorResult = DMG_PALETTE[m_dmgPalette->backgroundPalette[colorValue]];
							else if (colorPalette == 0)
								colorResult = DMG_PALETTE[m_dmgPalette->objectPalette0[colorValue]];
							else if (colorPalette == 1)
								colorResult = DMG_PALETTE[m_dmgPalette->objectPalette1[colorValue]];
						}

						// CGB mode : Resolve with CGB palettes (BCPD / OCPD)
						else if (m_hardware.mode() == OperationMode::CGB) {
							// Calculate the index in the palette data array
							int paletteIndex = colorPalette * 4 + colorValue;
							if (elementIndex == BACKGROUND_INDEX)
								colorResult = m_cgbPalette->backgroundPalettes[paletteIndex];
							else
								colorResult = m_cgbPalette->objectPalettes[paletteIndex];
						}

						// Finally render the pixel into the back pixels buffer
						m_backBuffer[line * LCD_WIDTH + x] = colorResult;

						wasInsideWindow = insideWindow;
					}

					// Mode 0 = HBlank
					m_lcdControl->modeFlag = 0;
					m_oamMapping->accessible = true;
					m_vramMapping->accessible = true;
					if (m_cgbPalette != nullptr)
						m_cgbPalette->accessible = true;

					if (m_lcdControl->hblankInterrupt){
						m_interrupt->setRequest(Interrupt::LCDStat);
					}

					for (int i = 0; i < hblankDuration; i++)
						dot();  // TODO

					// If the window was rendered at some point in the current scanline, increment the window line counter
					if (hasWindow)
						windowLineCounter += 1;
				}

				// Mode 1 = VBlank
				m_lcdControl->modeFlag = 1;
				m_oamMapping->accessible = true;
				m_vramMapping->accessible = true;
				if (m_cgbPalette != nullptr)
					m_cgbPalette->accessible = true;

				m_interrupt->setRequest(Interrupt::VBlank);

				// Off-screen scanlines (144-153)
				for (int line = 144; line < 153; line++) {
					// LY and LY = LYC STAT interrupts still update during VBlank
					m_lcdControl->coordY = line;
					if (m_lcdControl->coordY == m_lcdControl->coordYCompare && m_lcdControl->lycInterrupt)
						m_interrupt->setRequest(Interrupt::LCDStat);

					// Every line takes 456 clocks
					for (int i = 0; i < 456; i++)
						dot();  // TODO
				}

				// FIXME : Not sure about whether the interrupt request must be reset at the end of VBlank, common sense tells it should be but 80s hardware is not reknowned to have one
				m_interrupt->resetRequest(Interrupt::VBlank);
			} else {
				dot();
			}
		}
	}

	// Tell whether the emulator can skip running this component for the cycle, to save a context commutation if running it is useless
	bool LCDController::skip() const {
		// Skip if the PPU and LCD are disabled
		return !m_lcdControl->displayEnable;
	}

	// Return a full pixel buffer in RGB555 format
	uint16_t* LCDController::pixels() {
		return m_frontBuffer;
	}


	////////// LCDController::ObjectSelectionComparator
	// FIXME : Vestigial parameters

	LCDController::ObjectSelectionComparator::ObjectSelectionComparator(HardwareConfig& hardware, LCDMemoryMapping* oam) {
		m_hardware = hardware;
		m_oamMapping = oam;
	}

	// Sort in order of increasing X coordinate
	bool LCDController::ObjectSelectionComparator::operator()(const uint16_t& address1, const uint16_t& address2) {
		return m_oamMapping->lcdGet(address1 + 1) < m_oamMapping->lcdGet(address2 + 1);
	}


	// LCDController::Pixel

	LCDController::Pixel::Pixel(uint8_t pcolor, uint8_t ppalette, uint16_t paddress, bool ppriority) {
		color = pcolor;
		palette = ppalette;
		oamAddress = paddress;
		priority = ppriority;
	}
}
