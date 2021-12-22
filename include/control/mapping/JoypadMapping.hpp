#ifndef _MEMORY_MAPPING_JOYPADMAPPING_HPP
#define _MEMORY_MAPPING_JOYPADMAPPING_HPP

#include "util/enum.hpp"
#include "memory/MemoryMapping.hpp"


namespace toygb {
	/** Enumerate joypad buttons
	 * The integer values are arbitrary and just used as indices in the JoypadMapping::status array */
	enum class JoypadButton: int {
		Up    = 0, Down   = 1,
		Left  = 2, Right  = 3,
		A     = 4, B      = 5,
		Start = 6, Select = 7,
	};

	class JoypadMapping : public MemoryMapping {
		public:
			JoypadMapping();

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			/** Set a button status */
			void setButton(JoypadButton button, bool pressed);

			// Warning : all these bits’ logic is 0 = selected/pressed, 1 = not selected/not pressed
			bool selectButtons;     // Select action buttons row (register JOYP, bit 5)
			bool selectDirections;  // Select direction buttons row (register JOYP, bit 4)

			bool status[8];         // Buttons’ statuses, used to compose register JOYP, bits 0-3
	};
}

#endif
