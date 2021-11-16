#ifndef _CART_MAPPING_MBC1RAMMAPPING_HPP
#define _CART_MAPPING_MBC1RAMMAPPING_HPP

#include "memory/mapping/FullBankedMemoryMapping.hpp"


namespace toygb {
	class MBC1RAMMapping : public FullBankedMemoryMapping {
		public:
			MBC1RAMMapping(uint8_t* bankSelect, bool* modeSelect, uint16_t bankSize, uint8_t* array, bool accessible);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

		protected:
			bool* m_modeSelect;
	};
}

#endif
