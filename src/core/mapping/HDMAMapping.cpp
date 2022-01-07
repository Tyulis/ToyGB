#include "core/mapping/HDMAMapping.hpp"

#define OFFSET_START IO_HDMA_SOURCELOW
#define OFFSET_SOURCELOW  IO_HDMA_SOURCELOW - OFFSET_START
#define OFFSET_SOURCEHIGH IO_HDMA_SOURCEHIGH - OFFSET_START
#define OFFSET_DESTLOW    IO_HDMA_DESTLOW - OFFSET_START
#define OFFSET_DESTHIGH   IO_HDMA_DESTHIGH - OFFSET_START
#define OFFSET_SETTINGS   IO_HDMA_SETTINGS - OFFSET_START

/** Tone channel with frequency sweep control IO registers mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name  | Access   | Content
      FF51 |       0000 | HDMA1 | BBBBBBBB | Lower byte of the source address
      FF52 |       0001 | HDMA2 | BBBBBBBB | Upper byte of the source address
      FF53 |       0002 | HDMA3 | BBBBBBBB | Lower byte of the destination address
      FF54 |       0003 | HDMA4 | BBBBBBBB | Upper byte of the destination address
      FF55 |       0004 | HDMA5 | WBBBBBBB | HDMA control register : MLLLLLLL
           |            |       |          | - M (bit 7) : Write : Transfer mode (0 = general-purpose, 1 = HBlank transfer)
           |            |       |          |               Read  : Activity flag (0 = no active transfer, 1 = active)
           |            |       |          | - L (bits 0-6) : Write : Length of the data to transfer / 0x10 - 1
           |            |       |          |                  Read  : Length of the data remaining to transfer / 0x10 - 1 */


namespace toygb {
	// Initialize the memory mapping with its initial values
	HDMAMapping::HDMAMapping() {
		source = 0x0000;
		dest = 0x0000;
		type = false;
		length = 0x0000;
		active = false;
	}

	// Get the value at the given relative address
	uint8_t HDMAMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_SOURCELOW: return source & 0xFF;
			case OFFSET_SOURCEHIGH: return source >> 8;
			case OFFSET_DESTLOW: return dest & 0xFF;
			case OFFSET_DESTHIGH: return dest >> 8;
			case OFFSET_SETTINGS: return (active << 7) | (length / 0x10 - 1);
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	// Set the value at the given relative address (this is a stub)
	void HDMAMapping::set(uint16_t address, uint8_t value) {
		switch (address) {
			case OFFSET_SOURCELOW: source = (source & 0xFF00) | value; break;
			case OFFSET_SOURCEHIGH: source = (source & 0x00FF) | (value << 8); break;
			case OFFSET_DESTLOW: dest = (dest & 0xFF00) | value; break;
			case OFFSET_DESTHIGH: dest = (dest & 0x00FF) & (value << 8); break;
			case OFFSET_SETTINGS:
				type = value >> 7;
				length = ((value & 0x7F) + 1) * 0x10;
				active = true;
				break;
		}
	}
}
