#include "core/mapping/HDMAMapping.hpp"

#define OFFSET_START IO_HDMA_SOURCEHIGH
#define OFFSET_SOURCEHIGH IO_HDMA_SOURCEHIGH - OFFSET_START
#define OFFSET_SOURCELOW  IO_HDMA_SOURCELOW - OFFSET_START
#define OFFSET_DESTHIGH   IO_HDMA_DESTHIGH - OFFSET_START
#define OFFSET_DESTLOW    IO_HDMA_DESTLOW - OFFSET_START
#define OFFSET_SETTINGS   IO_HDMA_SETTINGS - OFFSET_START

/** Tone channel with frequency sweep control IO registers mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name  | Access   | Content
      FF51 |       0000 | HDMA1 | WWWWWWWW | Upper byte of the source address
      FF52 |       0001 | HDMA2 | WWWWWWWW | Lower byte of the source address
      FF53 |       0002 | HDMA3 | WWWWWWWW | Upper byte of the destination address
      FF54 |       0003 | HDMA4 | WWWWWWWW | Lower byte of the destination address
      FF55 |       0004 | HDMA5 | WBBBBBBB | HDMA control register : MLLLLLLL
           |            |       |          | - M (bit 7) : Write : Transfer mode (0 = general-purpose, 1 = HBlank transfer)
           |            |       |          |               Read  : Activity flag (0 = no active transfer, 1 = active)
           |            |       |          |               Write with HBlank DMA active : Transfer control (0 = stop, 1 = restart)
           |            |       |          | - L (bits 0-6) : Write : Number of 16-bytes blocks to transfer - 1
           |            |       |          |                  Read  : Number of 16-bytes blocks remaining to transfer - 1 */


namespace toygb {
	// Initialize the memory mapping with its initial values
	HDMAMapping::HDMAMapping() {
		source = 0x0000;
		dest = 0x0000;
		type = false;
		blocks = 0x00;
		active = false;
		paused = false;
	}

	// Get the value at the given relative address
	uint8_t HDMAMapping::get(uint16_t address) {
		switch (address) {
			// All those are write-only
			case OFFSET_SOURCEHIGH:  // HDMA1
			case OFFSET_SOURCELOW:   // HDMA2
			case OFFSET_DESTHIGH:    // HDMA3
			case OFFSET_DESTLOW:     // HDMA4
				return 0xFF;
			case OFFSET_SETTINGS:    // HDMA5
				return (active << 7) | ((blocks - 1) & 0x7F);
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	// Set the value at the given relative address (this is a stub)
	void HDMAMapping::set(uint16_t address, uint8_t value) {
		switch (address) {
			case OFFSET_SOURCEHIGH:  // HDMA1
				if ((value & 0xE0) == 0xE0)  // Source addresses in 0xE000-0xFFFF read from 0xA000-0xBFFF instead
					value = (value & 0x1F) | 0xA0;
				source = (source & 0x00FF) | (value << 8);
				break;
			case OFFSET_SOURCELOW:  // HDMA2 (lower 4 bits are ignored, as blocks are 16-bytes aligned)
				source = (source & 0xFF00) | (value & 0xF0);
				break;
			case OFFSET_DESTHIGH:   // HDMA3 (destination is always in VRAM, higher 3 bits are ignored)
				dest = (dest & 0x00FF) & (((value & 0x1F) | 0x80) << 8);
				break;
			case OFFSET_DESTLOW:    // HDMA4 (lower 4 bits are ignored, as blocks are 16-bytes aligned)
				dest = (dest & 0xFF00) | (value & 0xF0);
				break;
			case OFFSET_SETTINGS:
				if (active)
					paused = (value >> 7) & 1;
				else
					type = (value >> 7) & 1;
				blocks = (value & 0x7F) + 1;
				active = true;
				break;
		}
	}

	bool HDMAMapping::running() const {
		return blocks > 0 && active && !paused;
	}

	void HDMAMapping::nextBlock() {
		source += 0x0010;
		dest += 0x0010;
		blocks -= 1;

		// Source addresses in 0xE000-0xFFFF read from 0xA000-0xBFFF instead
		if ((source & 0xE000) == 0xE000)
			source = (source & 0x1FFF) | 0xA000;
	}
}
