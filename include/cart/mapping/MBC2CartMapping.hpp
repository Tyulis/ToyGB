#ifndef _CART_MAPPING_MBC2CARTMAPPING_HPP
#define _CART_MAPPING_MBC2CARTMAPPING_HPP

#include "cart/ROMMapping.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"


namespace toygb {
	/** MBC2 ROM memory mapping
	 * TODO : not implemented */
	class MBC2CartMapping : public ROMMapping {
		public:
			/** Initialize the mapping
			 * uint8_t carttype : Cart type identifier, as read at address 0x0147 of the cartridge header
			 * string romfile   : ROM file name
			 * string ramfile   : Save file name */
			MBC2CartMapping(uint8_t carttype, std::string romfile, std::string ramfile);
			virtual ~MBC2CartMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			/** Return the associated cartridge RAM memory mapping */
			virtual MemoryMapping* getRAM();

		protected:
			ArrayMemoryMapping* m_ramMapping;  // Associated cart RAM mapping
	};
}

#endif
