#ifndef _MEMORY_MAPPING_FIXBANKEDWRAMMAPPING_HPP
#define _MEMORY_MAPPING_FIXBANKEDWRAMMAPPING_HPP

#include "memory/mapping/BankedMemoryMapping.hpp"


namespace toygb {
	class FixBankedMemoryMapping : public BankedMemoryMapping {
		public:
			FixBankedMemoryMapping(uint8_t* bankSelect, int numBanks, uint16_t bankSize, uint8_t* array, bool accessible);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);
	};
}

#endif
