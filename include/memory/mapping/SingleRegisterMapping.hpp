#ifndef _MEMORY_MAPPING_SINGLEREGISTERMAPPING_HPP
#define _MEMORY_MAPPING_SINGLEREGISTERMAPPING_HPP

#include "memory/MemoryMapping.hpp"

namespace toygb {
	class SingleRegisterMapping : public MemoryMapping {
		public:
			SingleRegisterMapping(uint8_t* reg);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

		private:
			uint8_t* m_register;
	};
}

#endif
