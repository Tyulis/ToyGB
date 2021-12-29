#ifndef _CORE_MAPPING_BOOTROMDISABLEMAPPING_HPP
#define _CORE_MAPPING_BOOTROMDISABLEMAPPING_HPP

#include "memory/MemoryMapping.hpp"

namespace toygb {
	/** WRAM bank selection IO register memory mapping */
	class BootromDisableMapping : public MemoryMapping {
		public:
			BootromDisableMapping(bool* reg);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

		protected:
			bool* m_register;  // Register to put the value into
	};
}

#endif
