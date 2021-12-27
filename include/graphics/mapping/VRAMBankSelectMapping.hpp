#ifndef _GRAPHICS_MAPPING_VRAMBANKSELECTMAPPING_HPP
#define _GRAPHICS_MAPPING_VRAMBANKSELECTMAPPING_HPP

#include "memory/MemoryMapping.hpp"

namespace toygb {
	/** VRAM bank selection register memory mapping */
	class VRAMBankSelectMapping : public MemoryMapping {
		public:
			VRAMBankSelectMapping(uint8_t* reg);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

		protected:
			uint8_t* m_register;
	};
}

#endif
