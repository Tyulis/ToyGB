#ifndef _CART_CARTCONTROLLER_HPP
#define _CART_CARTCONTROLLER_HPP

#include <fstream>
#include <string>

#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"

#include "cart/ROMMapping.hpp"
#include "cart/mapping/ROMCartMapping.hpp"
#include "cart/mapping/MBC1CartMapping.hpp"
#include "cart/mapping/MBC2CartMapping.hpp"
#include "cart/mapping/MBC3CartMapping.hpp"
#include "cart/mapping/MBC4CartMapping.hpp"
#include "cart/mapping/MBC5CartMapping.hpp"
#include "cart/mapping/MMM01CartMapping.hpp"


namespace toygb {
	/** Cartridge controller, manages the cartridge content
	 * and its interface with ROM and save files */
	class CartController {
		public:
			CartController();
			~CartController();

			/** Initialize the cartridge with a ROM file and a save file (may be an empty string) */
			void init(std::string romfile, std::string ramfile, HardwareStatus* hardware);
			void configureMemory(MemoryMap* memory);

			/** Return an automatic hardware configuration to run the cartridge, based on the ROM header */
			HardwareStatus getDefaultHardwareStatus() const;

			/** Save the cartridge RAM to the predefined save file */
			void save();

			/** Feature checks */
			bool hasRAM() const;      // Check whether the cartridge contains RAM
			bool hasBattery() const;  // Check whether the cartridge has a battery (= saves its RAM)
			bool hasRTC() const;      // Check whether the cartridge has a Real-Time Clock

			/** Update the cartridge status, like the RTC */
			void update();

		private:
			std::string m_romfile;
			std::string m_ramfile;
			ROMMapping* m_romMapping;
			MemoryMapping* m_ramMapping;
			HardwareStatus* m_hardware;
	};
}

#endif
