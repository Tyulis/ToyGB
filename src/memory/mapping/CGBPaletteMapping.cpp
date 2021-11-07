#include "memory/mapping/CGBPaletteMapping.hpp"

#define OFFSET_START IO_BGPALETTE_INDEX
#define OFFSET_BGINDEX  IO_BGPALETTE_INDEX - OFFSET_START
#define OFFSET_BGDATA   IO_BGPALETTE_DATA - OFFSET_START
#define OFFSET_OBJINDEX IO_OBJPALETTE_INDEX - OFFSET_START
#define OFFSET_OBJDATA  IO_OBJPALETTE_DATA - OFFSET_START

namespace toygb {
	CGBPaletteMapping::CGBPaletteMapping(){
		accessible = true;

		objectIndex = 0;  // ?
		objectAutoIncrement = false;  // ?
		for (int i = 0; i < 8; i++){
			objectPalettes[i] = 0xFFFE;
		}

		backgroundIndex = 0;  // ?
		backgroundAutoIncrement = false;  // ?
		for (int i = 0; i < 8; i++){
			backgroundPalettes[i] = 0xFFFE;
		}
	}

	uint8_t CGBPaletteMapping::get(uint16_t address){
		if (!accessible) return 0xFF;

		switch (address) {
			case OFFSET_BGINDEX: return (backgroundAutoIncrement << 7) | backgroundIndex;
			case OFFSET_BGDATA: {
				uint16_t color = backgroundPalettes[backgroundIndex / 2];
				if (backgroundIndex % 2) {
					return color >> 8;
				} else {
					return color & 0xFF;
				}
			}
			case OFFSET_OBJINDEX: return (objectAutoIncrement << 7) | backgroundIndex;
			case OFFSET_OBJDATA: {
				uint16_t color = objectPalettes[objectIndex / 2];
				if (objectIndex % 2) {
					return color >> 8;
				} else {
					return color & 0xFF;
				}
			}
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	void CGBPaletteMapping::set(uint16_t address, uint8_t value){
		if (accessible) {
			switch (address) {
				case OFFSET_BGINDEX:
					backgroundAutoIncrement = value >> 7;
					backgroundIndex = value & 0x3F;
					break;
				case OFFSET_BGDATA: {
					int colorIndex = backgroundIndex / 2;
					if (backgroundIndex % 2) {
						backgroundPalettes[colorIndex] = (backgroundPalettes[colorIndex] & 0x00FF) | (value << 8);
					} else {
						backgroundPalettes[colorIndex] = (backgroundPalettes[colorIndex] & 0xFF00) | value;
					}
					if (backgroundAutoIncrement) backgroundIndex = (backgroundIndex + 1) & 0x3F;
					break;
				}
				case OFFSET_OBJINDEX:
					objectAutoIncrement = value >> 7;
					objectIndex = value & 0x3F;
					break;
				case OFFSET_OBJDATA: {
					int colorIndex = objectIndex / 2;
					if (objectIndex % 2) {
						objectPalettes[colorIndex] = (objectPalettes[colorIndex] & 0x00FF) | (value << 8);
					} else {
						objectPalettes[colorIndex] = (objectPalettes[colorIndex] & 0xFF00) | value;
					}
					if (objectAutoIncrement) objectIndex = (objectIndex + 1) & 0x3F;
					break;
				}
			}
		}
	}
}
