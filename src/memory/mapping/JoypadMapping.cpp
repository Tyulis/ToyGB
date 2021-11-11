#include "memory/mapping/JoypadMapping.hpp"
#include <iostream>
#include "util/error.hpp"


namespace toygb {
	JoypadMapping::JoypadMapping(){
		selectButtons = false;
		selectDirections = false;

		for (int i = 0; i < 8; i++) status[i] = false;
	}

	uint8_t JoypadMapping::get(uint16_t address) {
		uint8_t result = 0xC0 | (!selectButtons << 5) | (!selectDirections << 4);
		if (selectButtons) {
			result |= (!status[enumval(JoypadButton::Start)] << 3) | (!status[enumval(JoypadButton::Select)] << 2) | (!status[enumval(JoypadButton::B)] << 1) | (!status[enumval(JoypadButton::A)]);
		} else if (selectDirections) {
			result |= (!status[enumval(JoypadButton::Down)] << 3) | (!status[enumval(JoypadButton::Up)] << 2) | (!status[enumval(JoypadButton::Left)] << 1) | (!status[enumval(JoypadButton::Right)]);
		}
		//std::cout << "Joypad get : " << oh8(result) << std::endl;
		return result;
	}

	void JoypadMapping::set(uint16_t address, uint8_t value){
		//std::cout << "Joypad set : " << oh8(value) << std::endl;
		selectButtons = !((value >> 5) & 1);
		selectDirections = !((value >> 4) & 1);
	}

	void JoypadMapping::setButton(JoypadButton button, bool pressed){
		status[enumval(button)] = pressed;
	}
}
