#ifndef _CART_MAPPING_MBC1CARTMAPPING_HPP
#define _CART_MAPPING_MBC1CARTMAPPING_HPP

#include "cart/ROMMapping.hpp"
#include "cart/mapping/MBC1RAMMapping.hpp"
#include "memory/Constants.hpp"


namespace toygb {
	class MBC1CartMapping : public ROMMapping {
		public:
			MBC1CartMapping(uint8_t carttype, std::string romfile, std::string ramfile);
			virtual ~MBC1CartMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			virtual MemoryMapping* getRAM();

		protected:
			MBC1RAMMapping* m_ramMapping;
			uint8_t m_romBankSelect;
			uint8_t m_ramBankSelect;
			uint8_t m_romBankMask;
			bool m_modeSelect;
			int m_romBanks;
			int m_ramBanks;
	};
}

#endif
