#ifndef _CART_MAPPING_MMM01CARTMAPPING_HPP
#define _CART_MAPPING_MMM01CARTMAPPING_HPP

#include "cart/ROMMapping.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"


namespace toygb {
	/** MMM01 ROM memory mapping
	 * TODO : Not implemented */
	class MMM01CartMapping : public ROMMapping {
		public:
			/** Initialize the mapping
			 * uint8_t carttype : Cart type identifier, as read at address 0x0147 of the cartridge header
			 * string romfile   : ROM file name
			 * string ramfile   : Save file name */
			MMM01CartMapping(uint8_t carttype, std::string romfile, std::string ramfile, HardwareStatus* hardware);
			virtual ~MMM01CartMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			/** Return the associated cartridge RAM mapping */
			virtual MemoryMapping* getRAM();

		protected:
			ArrayMemoryMapping* m_ramMapping;  // Associated cart RAM mapping
	};
}

#endif
