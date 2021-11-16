#ifndef _MEMORY_MAPPING_FULLBANKEDWRAMMAPPING_HPP
#define _MEMORY_MAPPING_FULLBANKEDWRAMMAPPING_HPP

#include "memory/mapping/BankedMemoryMapping.hpp"


namespace toygb {
	class FullBankedMemoryMapping : public BankedMemoryMapping {
		public:
			FullBankedMemoryMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array, bool accessible);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);
	};
}

#endif
