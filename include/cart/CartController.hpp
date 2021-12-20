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
	class CartController {
		public:
			CartController();
			~CartController();

			void init(std::string romfile, std::string ramfile);
			void configureMemory(MemoryMap* memory);
			HardwareConfig getDefaultHardwareConfig();
			void save();

		private:
			std::string m_romfile;
			std::string m_ramfile;
			ROMMapping* m_romMapping;
			MemoryMapping* m_ramMapping;
	};
}

#endif
