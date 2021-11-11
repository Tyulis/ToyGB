#ifndef _MEMORY_MAPPING_JOYPADMAPPING_HPP
#define _MEMORY_MAPPING_JOYPADMAPPING_HPP

#include "util/enum.hpp"
#include "memory/MemoryMapping.hpp"


namespace toygb {
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

			void setButton(JoypadButton button, bool pressed);

			bool selectButtons;
			bool selectDirections;

			bool status[8];
	};
}

#endif
