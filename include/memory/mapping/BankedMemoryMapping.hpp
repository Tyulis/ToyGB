#ifndef _MEMORY_MAPPING_BANKEDWRAMMAPPING_HPP
#define _MEMORY_MAPPING_BANKEDWRAMMAPPING_HPP

#include "memory/MemoryMapping.hpp"


namespace toygb {
	class BankedMemoryMapping : public MemoryMapping {
		public:
			BankedMemoryMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array, bool accessible);

			virtual uint8_t get(uint16_t address) = 0;
			virtual void set(uint16_t address, uint8_t value) = 0;

			bool accessible;

		protected:
			uint8_t* m_bankSelect;
			uint16_t m_bankSize;
			uint8_t* m_array;
	};
}

#endif
