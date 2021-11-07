#ifndef _MEMORY_MAPPING_HDMAMAPPING_HPP
#define _MEMORY_MAPPING_HDMAMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class HDMAMapping : public MemoryMapping {
		public:
			HDMAMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			uint16_t source;
			uint16_t dest;
			bool type;
			uint16_t length;
			bool active;
	};
}

#endif
