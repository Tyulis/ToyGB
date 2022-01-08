#ifndef _CART_MAPPING_MBC1CARTMAPPING_HPP
#define _CART_MAPPING_MBC1CARTMAPPING_HPP

#include <cmath>

#include "cart/ROMMapping.hpp"
#include "cart/mapping/MBC1RAMMapping.hpp"
#include "memory/Constants.hpp"


namespace toygb {
	/** MBC1 ROM memory mapping */
	class MBC1CartMapping : public ROMMapping {
		public:
			/** Initialize the mapping
			 * uint8_t carttype : Cart type identifier, as read at address 0x0147 of the cartridge header
			 * string romfile   : ROM file name
			 * string ramfile   : Save file name */
			MBC1CartMapping(uint8_t carttype, std::string romfile, std::string ramfile, HardwareStatus* hardware);
			virtual ~MBC1CartMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			/** Return the associated cartridge RAM memory mapping */
			virtual MemoryMapping* getRAM();

		protected:
			MBC1RAMMapping* m_ramMapping;  // Associated cart RAM mapping

			int m_romBanks;
			int m_ramBanks;

			// Internal registers
			uint8_t m_romBankSelect;
			uint8_t m_ramBankSelect;
			uint8_t m_romBankMask;
			bool m_modeSelect;
	};
}

#endif
