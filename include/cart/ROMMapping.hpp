#ifndef _CART_ROMMAPPING_HPP
#define _CART_ROMMAPPING_HPP

#include <string>
#include <fstream>

#include "core/hardware.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Base class for ROM memory mappings */
	class ROMMapping : public MemoryMapping {
		public:
			/** Initialize the mapping
			 * uint8_t carttype : Cart type identifier, as read at address 0x0147 of the cartridge header
			 * string romfile   : ROM file name
			 * string ramfile   : Save file name */
			ROMMapping(uint8_t carttype, std::string romfile, std::string ramfile);
			virtual ~ROMMapping();

			virtual uint8_t get(uint16_t address) = 0;
			virtual void set(uint16_t address, uint8_t value) = 0;

			/** Return the associated cartridge RAM mapping */
			virtual MemoryMapping* getRAM() = 0;

			/** Return an automatic hardware configuration to run the ROM, based on its header */
			HardwareStatus getDefaultHardwareStatus() const;

			bool hasRAM() const;  // Check whether the cartridge contains RAM
			bool hasBattery() const;  // Check whether the cartridge has a battery (= saves its RAM)

		protected:
			/** Set the cartridge features as defined by the cartridge type identifier in the ROM header, for use by subclasses. */
			void setCartFeatures(bool hasRAM, bool hasBattery);

			/** Load the cartridge data in memory, from the ROM file and the RAM file if it exists and if the cart has a battery */
			void loadCartData();

			std::string m_romFile;  // ROM file name
			std::string m_ramFile;  // Save file name

			int m_romSize;  // ROM size in bytes
			int m_ramSize;  // RAM size in bytes
			uint8_t* m_romData;  // Full ROM data
			uint8_t* m_ramData;  // Full RAM data
			uint8_t m_cartType;  // Cartridge type identifier, from 0x0147 in the ROM header

			bool m_hasRAM;
			bool m_hasBattery;
	};
}

#endif
