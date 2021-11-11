#ifndef _CART_MAPPING_MBC1CARTMAPPING_HPP
#define _CART_MAPPING_MBC1CARTMAPPING_HPP

#include "cart/ROMMapping.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"


namespace toygb {
	class MBC1CartMapping : public ROMMapping {
		public:
			MBC1CartMapping(uint8_t carttype, std::string romfile, std::string ramfile);
			virtual ~MBC1CartMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			virtual MemoryMapping* getRAM();

		protected:
			ArrayMemoryMapping* m_ramMapping;
	};
}

#endif
