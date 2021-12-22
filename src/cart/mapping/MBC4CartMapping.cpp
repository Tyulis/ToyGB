#include "cart/mapping/MBC4CartMapping.hpp"

#define CARTTYPE_MBC4 0x15
#define CARTTYPE_MBC4_RAM 0x16
#define CARTTYPE_MBC4_RAM_BATTERY 0x17

namespace toygb {
	// TODO : Not implemented
	MBC4CartMapping::MBC4CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {
		switch (carttype) {
			case CARTTYPE_MBC4: setCartFeatures(false, false); break;
			case CARTTYPE_MBC4_RAM: setCartFeatures(true, false); break;
			case CARTTYPE_MBC4_RAM_BATTERY: setCartFeatures(true, true); break;
		}

		loadCartData();

		if (m_ramData != nullptr) {
			m_ramMapping = new ArrayMemoryMapping(m_ramData);
		} else {
			m_ramMapping = nullptr;
		}
	}

	MBC4CartMapping::~MBC4CartMapping() {

	}

	MemoryMapping* MBC4CartMapping::getRAM() {
		return m_ramMapping;
	}

	uint8_t MBC4CartMapping::get(uint16_t address) {
		return m_romData[address];
	}

	void MBC4CartMapping::set(uint16_t address, uint8_t value) {
		// nop
	}
}
