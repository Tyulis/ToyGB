#ifndef _CART_MAPPING_MMM01CARTMAPPING_HPP
#define _CART_MAPPING_MMM01CARTMAPPING_HPP

#include "cart/ROMMapping.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"


namespace toygb {
	class MMM01CartMapping : public ROMMapping {
		public:
			MMM01CartMapping(uint8_t carttype, std::string romfile, std::string ramfile);
			virtual ~MMM01CartMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			virtual MemoryMapping* getRAM();

		protected:
			ArrayMemoryMapping* m_ramMapping;
	};
}

#endif
