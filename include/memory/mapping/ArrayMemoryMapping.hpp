#ifndef _MEMORY_MAPPING_ARRAYMEMORYMAPPING_HPP
#define _MEMORY_MAPPING_ARRAYMEMORYMAPPING_HPP

#include "memory/MemoryMapping.hpp"

namespace toygb {
	class ArrayMemoryMapping : public MemoryMapping {
		public:
			ArrayMemoryMapping(uint8_t* array);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

		protected:
			uint8_t* m_array;
	};
}

#endif
