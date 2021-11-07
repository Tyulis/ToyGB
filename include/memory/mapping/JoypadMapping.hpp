#ifndef _MEMORY_MAPPING_JOYPADMAPPING_HPP
#define _MEMORY_MAPPING_JOYPADMAPPING_HPP

#include "memory/MemoryMapping.hpp"


namespace toygb {
	class JoypadMapping : public MemoryMapping {
		public:
			JoypadMapping();

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			bool selectButtons;
			bool selectDirections;

			bool pressDown;
			bool pressUp;
			bool pressLeft;
			bool pressRight;

			bool pressStart;
			bool pressSelect;
			bool pressB;
			bool pressA;
	};
}

#endif
