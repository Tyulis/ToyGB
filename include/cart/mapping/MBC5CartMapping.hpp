#ifndef _CART_MAPPING_MBC5CARTMAPPING_HPP
#define _CART_MAPPING_MBC5CARTMAPPING_HPP

#include "cart/ROMMapping.hpp"
#include "memory/mapping/FullBankedMemoryMapping.hpp"


namespace toygb {
	/** MBC5 ROM memory mapping
	 * TODO : Not implemented */
	class MBC5CartMapping : public ROMMapping {
		public:
			/** Initialize the mapping
			 * uint8_t carttype : Cart type identifier, as read at address 0x0147 of the cartridge header
			 * string romfile   : ROM file name
			 * string ramfile   : Save file name */
			MBC5CartMapping(uint8_t carttype, std::string romfile, std::string ramfile, HardwareStatus* hardware);
			virtual ~MBC5CartMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			/** Return the associated cart RAM mapping */
			virtual MemoryMapping* getRAM();

		protected:
			FullBankedMemoryMapping* m_ramMapping;  // Associated cart RAM mapping

			int m_romBanks;
			int m_ramBanks;

			// Internal registers
			uint16_t m_romBankSelect;
			uint8_t m_ramBankSelect;
	};
}

#endif
