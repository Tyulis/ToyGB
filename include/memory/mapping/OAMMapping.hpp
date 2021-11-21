#ifndef _MEMORY_MAPPING_OAMMAPPING_HPP
#define _MEMORY_MAPPING_OAMMAPPING_HPP

#include "core/OperationMode.hpp"
#include "memory/Constants.hpp"
#include "memory/mapping/LCDMemoryMapping.hpp"

namespace toygb {
	class OAMMapping : public LCDMemoryMapping {
		public:
			OAMMapping(OperationMode mode, uint8_t* array);
			~OAMMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);
			virtual uint8_t lcdGet(uint16_t address);
			virtual void lcdSet(uint16_t address, uint8_t value);

		protected:
			OperationMode m_mode;
			uint8_t* m_fea0;
			uint8_t* m_fec0;
	};
}

#endif
