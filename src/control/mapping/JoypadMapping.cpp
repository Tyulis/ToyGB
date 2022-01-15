#include "control/mapping/JoypadMapping.hpp"


/** Joypad button status register

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name    | Access   | Content
      FF00 |       0000 | PI/JOYP | --BBRRRR | Joypad status matrix : --AD3210
           |            |         |          | - A (bit 5) : Select the action buttons row to appear in bits 0-3 (0 = selected, 1 = unselected)
           |            |         |          | - D (bit 4) : Select the directions buttons row (0 = selected, 1 = unselected)
           |            |         |          | Button read if | actions are selected | directions are selected | (0 = pressed, 1 = not pressed)
           |            |         |          | - 3 (bit 3) :  |                Start |                    Down | (P13 pin input)
           |            |         |          | - 2 (bit 2) :  |               Select |                      Up | (P12 pin input)
           |            |         |          | - 1 (bit 1) :  |                    B |                    Left | (P11 pin input)
           |            |         |          | - 0 (bit 0) :  |                    A |                   Right | (P10 pin input) */

namespace toygb {
	// Initialize the memory mapping
	JoypadMapping::JoypadMapping() {
		// Neither are selected (but 0 = selected, 1 = not)
		selectButtons = false;
		selectDirections = false;

		// Start all set. TODO : those may be different on CGB/SGB
		for (int i = 0; i < 8; i++)
			status[i] = true;
	}

	// Get the value at the given relative address
	// Warning : all the logic is inverted (0 = selected, 1 = not)
	uint8_t JoypadMapping::get(uint16_t address) {
		uint8_t result = 0xC0 | (selectButtons << 5) | (selectDirections << 4);
		if (selectButtons && selectDirections)  // Neither row is selected : all lower bits set (as in nothing selected)
			result |= 0x0F;

		if (!selectButtons)  // Buttons selected
			result |= (status[enumval(JoypadButton::Start)] << 3) | (status[enumval(JoypadButton::Select)] << 2) | (status[enumval(JoypadButton::B)] << 1) | (status[enumval(JoypadButton::A)]);
		if (!selectDirections)  // Directions selected
			result |= (status[enumval(JoypadButton::Down)] << 3) | (status[enumval(JoypadButton::Up)] << 2) | (status[enumval(JoypadButton::Left)] << 1) | (status[enumval(JoypadButton::Right)]);
		// If both rows are selected, both rows’ statuses are OR-ed together, so this does the trick

		return result;
	}

	// Set the value at the given relative address
	void JoypadMapping::set(uint16_t address, uint8_t value) {
		selectButtons = ((value >> 5) & 1);
		selectDirections = ((value >> 4) & 1);
	}

	// Set a button status from the UI
	void JoypadMapping::setButton(JoypadButton button, bool pressed) {
		status[enumval(button)] = !pressed;  // Inverted, we take a logical argument (1 = pressed), and convert it to our register’s logic (0 = pressed)
	}
}
