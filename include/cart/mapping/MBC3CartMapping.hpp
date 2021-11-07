#ifndef _CART_MAPPING_MBC3CARTMAPPING_HPP
#define _CART_MAPPING_MBC3CARTMAPPING_HPP

#include "cart/ROMMapping.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"


namespace toygb {
	class MBC3CartMapping : public ROMMapping {
		public:
			MBC3CartMapping(uint8_t carttype, std::string romfile, std::string ramfile);
			virtual ~MBC3CartMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			virtual MemoryMapping* getSRAM();

		protected:
			ArrayMemoryMapping* m_sramMapping;
	};
}

#endif
