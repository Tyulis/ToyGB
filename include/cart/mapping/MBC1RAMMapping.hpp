#ifndef _CART_MAPPING_MBC1RAMMAPPING_HPP
#define _CART_MAPPING_MBC1RAMMAPPING_HPP

#include "memory/mapping/FullBankedMemoryMapping.hpp"


namespace toygb {
	/** MBC1 SRAM memory mapping */
	class MBC1RAMMapping : public FullBankedMemoryMapping {
		public:
			/** Initialize the memory mapping
			 * uint8_t* bankSelect : Pointer to the RAM bank select register
			 * bool* modeSelect    : Pointer to the banking mode select register
			 * int numBanks        : Total amount of RAM banks
			 * uint16_t bankSize   : Size of one RAM bank, in bytes
			 * uint8_t* array      : RAM content, must be of size (numBanks * bankSize)
			 * bool accessible     : Whether the memory mapping is accessible initially */
			MBC1RAMMapping(uint8_t* bankSelect, bool* modeSelect, int numBanks, uint16_t bankSize, uint8_t* array, bool accessible);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

		protected:
			bool* m_modeSelect;
	};
}

#endif
