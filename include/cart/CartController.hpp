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
			void init(std::string romfile, std::string ramfile);
			void configureMemory(MemoryMap* memory);

			/** Return an automatic hardware configuration to run the cartridge, based on the ROM header */
			HardwareConfig getDefaultHardwareConfig() const;

			/** Save the cartridge RAM to the predefined save file */
			void save();

		private:
			std::string m_romfile;
			std::string m_ramfile;
			ROMMapping* m_romMapping;
			MemoryMapping* m_ramMapping;
	};
}

#endif
