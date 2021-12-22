#include "cart/mapping/MMM01CartMapping.hpp"

#define CARTTYPE_MMM01 0x0B
#define CARTTYPE_MMM01_RAM 0x0C
#define CARTTYPE_MMM01_RAM_BATTERY 0x0D

namespace toygb {
	// TODO : Not implemented
	MMM01CartMapping::MMM01CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {
		switch (carttype){
			case CARTTYPE_MMM01: setCartFeatures(false, false); break;
			case CARTTYPE_MMM01_RAM: setCartFeatures(true, false); break;
			case CARTTYPE_MMM01_RAM_BATTERY: setCartFeatures(true, true); break;
		}

		loadCartData();

		if (m_ramData != nullptr) {
			m_ramMapping = new ArrayMemoryMapping(m_ramData);
		} else {
			m_ramMapping = nullptr;
		}
	}

	MMM01CartMapping::~MMM01CartMapping() {

	}

	MemoryMapping* MMM01CartMapping::getRAM() {
		return m_ramMapping;
	}

	uint8_t MMM01CartMapping::get(uint16_t address) {
		return m_romData[address];
	}

	void MMM01CartMapping::set(uint16_t address, uint8_t value) {
		// nop
	}
}
