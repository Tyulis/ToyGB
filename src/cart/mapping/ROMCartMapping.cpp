#include "cart/mapping/ROMCartMapping.hpp"

#define CARTTYPE_ROM 0x00
#define CARTTYPE_ROM_RAM 0x08
#define CARTTYPE_ROM_RAM_BATTERY 0x09

namespace toygb {
	// Initialize the memory mapping
	ROMCartMapping::ROMCartMapping(uint8_t carttype, std::string romfile, std::string ramfile, HardwareStatus* hardware) : ROMMapping(carttype, romfile, ramfile, hardware) {
		switch (carttype){
			case CARTTYPE_ROM: setCartFeatures(false, false, false); break;
			case CARTTYPE_ROM_RAM: setCartFeatures(true, false, false); break;
			case CARTTYPE_ROM_RAM_BATTERY: setCartFeatures(true, true, false); break;
		}

		loadCartData();

		if (m_ramData != nullptr) {
			m_ramMapping = new ArrayMemoryMapping(m_ramData);
			loadSaveData(m_ramMapping);
		} else {
			m_ramMapping = nullptr;
		}
	}

	ROMCartMapping::~ROMCartMapping() {

	}



	MemoryMapping* ROMCartMapping::getRAM() {
		return m_ramMapping;
	}

	uint8_t ROMCartMapping::get(uint16_t address) {
		return m_romData[address];
	}

	void ROMCartMapping::set(uint16_t address, uint8_t value) {
		// nop
	}
}
