#ifndef _CART_MAPPING_MBC3CARTMAPPING_HPP
#define _CART_MAPPING_MBC3CARTMAPPING_HPP

#include "cart/ROMMapping.hpp"
#include "memory/Constants.hpp"
#include "memory/mapping/FullBankedMemoryMapping.hpp"


namespace toygb {
	/** MBC3 ROM memory mapping */
	class MBC3CartMapping : public ROMMapping {
		public:
			/** Initialize the mapping
			 * uint8_t carttype : Cart type identifier, as read at address 0x0147 of the cartridge header
			 * string romfile   : ROM file name
			 * string ramfile   : Save file name */
			MBC3CartMapping(uint8_t carttype, std::string romfile, std::string ramfile);
			virtual ~MBC3CartMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			/** Return the associated cartridge RAM mapping */
			virtual MemoryMapping* getRAM();

		protected:
			uint8_t m_romBankSelect;
			uint8_t m_ramBankSelect;
			FullBankedMemoryMapping* m_ramMapping;  // Associated cart RAM mapping
	};
}

#endif
