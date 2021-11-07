#include "memory/mapping/JoypadMapping.hpp"


namespace toygb {
	JoypadMapping::JoypadMapping(){
		selectButtons = false;
		selectDirections = false;

		pressDown = false;
		pressUp = false;
		pressLeft = false;
		pressRight = false;

		pressStart = false;
		pressSelect = false;
		pressA = false;
		pressB = false;
	}

	uint8_t JoypadMapping::get(uint16_t address) {
		uint8_t result = 0xC0 | (!selectButtons << 5) | (!selectDirections << 4);
		if (selectButtons) {
			result |= (!pressStart << 3) | (!pressSelect << 2) | (!pressB << 1) | (!pressA);
		} else if (selectDirections) {
			result |= (!pressDown << 3) | (!pressUp << 2) | (!pressLeft << 1) | (!pressRight);
		}
		return result;
	}

	void JoypadMapping::set(uint16_t address, uint8_t value){
		selectButtons = !((value >> 5) & 1);
		selectDirections = !((value >> 4) & 1);
	}
}
