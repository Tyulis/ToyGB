#include "graphics/mapping/CGBPaletteMapping.hpp"

#define OFFSET_START IO_BGPALETTE_INDEX
#define OFFSET_BGINDEX     IO_BGPALETTE_INDEX - OFFSET_START
#define OFFSET_BGDATA      IO_BGPALETTE_DATA - OFFSET_START
#define OFFSET_OBJINDEX    IO_OBJPALETTE_INDEX - OFFSET_START
#define OFFSET_OBJDATA     IO_OBJPALETTE_DATA - OFFSET_START
#define OFFSET_OBJPRIORITY IO_OBJPRIORITY - OFFSET_START

/** CGB color palettes IO registers mapping
CGB palettes hold two blocks of 64 bytes, one for background palettes and one for object palettes
Each of those blocks is addressed, managed and used in the same way.
Each block is 64 bytes : 8 palettes of 4 little-endian RGB555 values (16-bits)
Colors are stored in little-endian RGB555 format, so a color value is 16-bits with bits 0-4 for red, 5-9 for green and 10-14 for blue.
In memory, a color looks like this (bit by bit) : | GGGRRRRR | -BBBBBGG |
FIXME : Not sure whether BCPS and OCPS are read-write or write-only (currently implemented as read-write)

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF68 |       0000 | BCPS | B-BBBBBB | Background palettes addressing control : A-IIIIII
           |            |      |          | - A (bit 7) : Auto-increment the current index after writing to BCPD
           |            |      |          | - I (bit 0-5) : Index of the background palette memory accessible from BCPD
      FF69 |       0001 | BCPD | BBBBBBBB | Addressed byte of the background palette memory, as set by BCPS.0-5
      FF6A |       0002 | OCPS | B-BBBBBB | Object palettes addressing control : A-IIIIII
           |            |      |          | - A (bit 7) : Auto-increment the current index after writing to OCPD
           |            |      |          | - I (bit 0-5) : Index of the object palette memory accessible from OCPD
      FF6B |       0003 | OCPD | BBBBBBBB | Addressed byte of the object palette memory, as set by OCPS.0-5
      FF6C |       0004 | OPRI | -------W | Objects priority mode control
           |            |      |          | If bit 0 is set to 0, the object with the lowest OAM index gets priority (CGB mode)
           |            |      |          | If it is set to 1, the object with the lowest X coordinate gets priority (DMG mode) */


namespace toygb {
	// Initialize the memory with its initial values
	CGBPaletteMapping::CGBPaletteMapping(HardwareStatus* hardware) {
		m_hardware = hardware;
		accessible = true;

		objectIndex = 0x3F;
		objectAutoIncrement = true;
		for (int i = 0; i < 8; i++)  // By default the bootrom initializes all palettes to white
			objectPalettes[i] = 0x7FFF;

		backgroundIndex = 0x3F;
		backgroundAutoIncrement = true;
		for (int i = 0; i < 8; i++)
			backgroundPalettes[i] = 0x7FFF;

		objectPriority = false;
	}

	// Get the value at the given relative address
	uint8_t CGBPaletteMapping::get(uint16_t address){
		if (!accessible) return 0xFF;

		switch (address) {
			case OFFSET_BGINDEX:  // BCPS
				return (backgroundAutoIncrement << 7) | backgroundIndex | 0x40;
			case OFFSET_BGDATA: {  // BCPD
				// Autoincrement is only for writes
				uint16_t color = backgroundPalettes[backgroundIndex >> 1];
				if (backgroundIndex & 1)
					return color >> 8;  // Little endian : upper byte last
				else
					return color & 0xFF;
			}
			case OFFSET_OBJINDEX:  // OCPS
				return (objectAutoIncrement << 7) | backgroundIndex | 0x40;
			case OFFSET_OBJDATA: {  // OCPD
				uint16_t color = objectPalettes[objectIndex / 2];
				if (objectIndex & 1)
					return color >> 8;
				else
					return color & 0xFF;
			}
			case OFFSET_OBJPRIORITY:  // OPRI
				return 0xFF;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	// Set the value at the given relative address
	void CGBPaletteMapping::set(uint16_t address, uint8_t value) {
		if (accessible) {
			switch (address) {
				case OFFSET_BGINDEX:  // BCPS
					backgroundAutoIncrement = value >> 7;
					backgroundIndex = value & 0x3F;
					break;
				case OFFSET_BGDATA: {  // BCPD
					int colorIndex = backgroundIndex >> 1;
					if (backgroundIndex & 1)  // Little endian : upper byte last
						backgroundPalettes[colorIndex] = (backgroundPalettes[colorIndex] & 0x00FF) | (value << 8);
					else
						backgroundPalettes[colorIndex] = (backgroundPalettes[colorIndex] & 0xFF00) | value;

					if (backgroundAutoIncrement)
						backgroundIndex = (backgroundIndex + 1) & 0x3F;
					break;
				}
				case OFFSET_OBJINDEX:  // OCPS
					objectAutoIncrement = value >> 7;
					objectIndex = value & 0x3F;
					break;
				case OFFSET_OBJDATA: {  // OCPD
					int colorIndex = objectIndex >> 1;
					if (objectIndex & 1)
						objectPalettes[colorIndex] = (objectPalettes[colorIndex] & 0x00FF) | (value << 8);
					else
						objectPalettes[colorIndex] = (objectPalettes[colorIndex] & 0xFF00) | value;

					if (objectAutoIncrement)
						objectIndex = (objectIndex + 1) & 0x3F;
					break;
				}
				case OFFSET_OBJPRIORITY:  // OPRI
					if (!m_hardware->bootromUnmapped())
						objectPriority = value & 1;
			}
		}
	}
}
