#ifndef _CART_ROMMAPPING_HPP
#define _CART_ROMMAPPING_HPP

#include <string>
#include <fstream>

#include "util/error.hpp"
#include "core/OperationMode.hpp"
#include "memory/MemoryMapping.hpp"


namespace toygb {
	class ROMMapping : public MemoryMapping {
		public:
			ROMMapping(uint8_t carttype, std::string romfile, std::string ramfile);
			virtual ~ROMMapping();

			virtual uint8_t get(uint16_t address) = 0;
			virtual void set(uint16_t address, uint8_t value) = 0;

			virtual MemoryMapping* getRAM() = 0;
			OperationMode getAutoOperationMode();
			bool hasRAM() const;
			bool hasBattery() const;

		protected:
			void setCartFeatures(bool hasRAM, bool hasBattery);
			void loadCartData();

			std::string m_romFile;
			std::string m_ramFile;
			int m_romSize;
			int m_ramSize;
			uint8_t* m_romData;
			uint8_t* m_ramData;
			uint8_t m_cartType;
			bool m_hasRAM;
			bool m_hasBattery;
	};
}

#endif
