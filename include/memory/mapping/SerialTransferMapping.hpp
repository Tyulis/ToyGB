#ifndef _MEMORY_MAPPING_SERIALTRANSFERMAPPING_HPP
#define _MEMORY_MAPPING_SERIALTRANSFERMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "core/OperationMode.hpp"
#include "util/error.hpp"


namespace toygb {
	class SerialTransferMapping : public MemoryMapping {
		public:
			SerialTransferMapping(OperationMode mode);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			uint8_t transferData;
			bool transferStartFlag;
			bool clockSpeed;
			bool shiftClock;
	};
}

#endif
