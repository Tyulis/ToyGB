#ifndef _CORE_MAPPING_WRAMBANKSELECTMAPPING_HPP
#define _CORE_MAPPING_WRAMBANKSELECTMAPPING_HPP

#include "memory/MemoryMapping.hpp"

namespace toygb {
	/** WRAM bank selection IO register memory mapping */
	class WRAMBankSelectMapping : public MemoryMapping {
		public:
			WRAMBankSelectMapping(uint8_t* reg);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

		protected:
			uint8_t* m_register;  // Register to put the value into
	};
}

#endif
