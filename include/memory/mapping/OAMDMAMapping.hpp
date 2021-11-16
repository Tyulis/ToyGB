#ifndef _MEMORY_MAPPING_OAMDMAMAPPING_HPP
#define _MEMORY_MAPPING_OAMDMAMAPPING_HPP

#include "core/OperationMode.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class OAMDMAMapping : public MemoryMapping {
		public:
			OAMDMAMapping(OperationMode mode);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			uint16_t sourceAddress;
			bool active;

		private:
			OperationMode m_mode;
	};
}

#endif
