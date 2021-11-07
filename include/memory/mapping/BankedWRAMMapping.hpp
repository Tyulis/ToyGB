#ifndef _MEMORY_MAPPING_BANKEDWRAMMAPPING_HPP
#define _MEMORY_MAPPING_BANKEDWRAMMAPPING_HPP

#include "memory/MemoryMapping.hpp"


namespace toygb {
	class BankedWRAMMapping : public MemoryMapping {
		public:
			BankedWRAMMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

		private:
			uint8_t* m_bankSelect;
			uint16_t m_bankSize;
			uint8_t* m_array;
	};
}

#endif
