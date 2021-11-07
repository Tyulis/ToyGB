#ifndef _CART_MAPPING_MBC5CARTMAPPING_HPP
#define _CART_MAPPING_MBC5CARTMAPPING_HPP

#include "cart/ROMMapping.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"


namespace toygb {
	class MBC5CartMapping : public ROMMapping {
		public:
			MBC5CartMapping(uint8_t carttype, std::string romfile, std::string ramfile);
			virtual ~MBC5CartMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			virtual MemoryMapping* getSRAM();

		protected:
			ArrayMemoryMapping* m_sramMapping;
	};
}

#endif
