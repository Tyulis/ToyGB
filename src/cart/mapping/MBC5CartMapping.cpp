#include "cart/mapping/MBC5CartMapping.hpp"

#define CARTTYPE_MBC5 0x19
#define CARTTYPE_MBC5_RAM 0x1A
#define CARTTYPE_MBC5_RAM_BATTERY 0x1B
#define CARTTYPE_MBC5_RUMBLE 0x1C
#define CARTTYPE_MBC5_RUMBLE_RAM 0x1D
#define CARTTYPE_MBC5_RUMBLE_RAM_BATTERY 0x1E

namespace toygb {
	MBC5CartMapping::MBC5CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {
		switch (carttype){
			case CARTTYPE_MBC5: setCartFeatures(false, false); break;
			case CARTTYPE_MBC5_RAM: setCartFeatures(true, false); break;
			case CARTTYPE_MBC5_RAM_BATTERY: setCartFeatures(true, true); break;
			case CARTTYPE_MBC5_RUMBLE: setCartFeatures(false, false); break;
			case CARTTYPE_MBC5_RUMBLE_RAM: setCartFeatures(true, false); break;
			case CARTTYPE_MBC5_RUMBLE_RAM_BATTERY: setCartFeatures(true, true); break;
		}

		loadCartData();

		if (m_ramData != nullptr){
			m_ramMapping = new ArrayMemoryMapping(m_ramData);
		} else {
			m_ramMapping = nullptr;
		}
	}

	MBC5CartMapping::~MBC5CartMapping(){

	}

	MemoryMapping* MBC5CartMapping::getRAM(){
		return m_ramMapping;
	}

	uint8_t MBC5CartMapping::get(uint16_t address){
		return m_romData[address];
	}

	void MBC5CartMapping::set(uint16_t address, uint8_t value){
		// nop
	}
}
