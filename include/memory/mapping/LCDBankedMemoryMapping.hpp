#ifndef _MEMORY_MAPPING_LCDBANKEDMEMORYMAPPING_HPP
#define _MEMORY_MAPPING_LCDBANKEDMEMORYMAPPING_HPP

#include "memory/mapping/LCDMemoryMapping.hpp"


namespace toygb {
	class LCDBankedMemoryMapping : public LCDMemoryMapping {
		public:
			LCDBankedMemoryMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);
			virtual uint8_t lcdGet(uint16_t address);
			virtual void lcdSet(uint16_t address, uint8_t value);

		private:
			uint8_t* m_bankSelect;
			uint16_t m_bankSize;
	};
}

#endif
