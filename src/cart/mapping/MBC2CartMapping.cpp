#include "cart/mapping/MBC2CartMapping.hpp"

#define CARTTYPE_MBC2 0x05
#define CARTTYPE_MBC2_BATTERY 0x06

namespace toygb {
	// TODO : Not implemented
	MBC2CartMapping::MBC2CartMapping(uint8_t carttype, std::string romfile, std::string ramfile, HardwareStatus* hardware) : ROMMapping(carttype, romfile, ramfile, hardware) {
		switch (carttype){
			case CARTTYPE_MBC2: setCartFeatures(false, false, false); break;
			case CARTTYPE_MBC2_BATTERY: setCartFeatures(false, true, false); break;
		}

		loadCartData();

		if (m_ramData != nullptr) {
			m_ramMapping = new ArrayMemoryMapping(m_ramData);
			loadSaveData(m_ramMapping);
		} else {
			m_ramMapping = nullptr;
		}
	}

	MBC2CartMapping::~MBC2CartMapping() {

	}

	MemoryMapping* MBC2CartMapping::getRAM() {
		return m_ramMapping;
	}

	uint8_t MBC2CartMapping::get(uint16_t address) {
		return m_romData[address];
	}

	void MBC2CartMapping::set(uint16_t address, uint8_t value) {
		// nop
	}
}
