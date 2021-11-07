#ifndef _MEMORY_MAPPING_LCDMEMORYMAPPING_HPP
#define _MEMORY_MAPPING_LCDMEMORYMAPPING_HPP

#include "memory/MemoryMapping.hpp"

namespace toygb {
	class LCDMemoryMapping : public MemoryMapping {
		public:
			LCDMemoryMapping(uint8_t* array);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			bool accessible;

		protected:
			uint8_t* m_array;
	};
}

#endif
