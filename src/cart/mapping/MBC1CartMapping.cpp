#include "cart/mapping/MBC1CartMapping.hpp"

#define CARTTYPE_MBC1 0x01
#define CARTTYPE_MBC1_RAM 0x02
#define CARTTYPE_MBC1_RAM_BATTERY 0x03

namespace toygb {
	MBC1CartMapping::MBC1CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile){
		switch (carttype){
			case CARTTYPE_MBC1: setCartFeatures(false, false); break;
			case CARTTYPE_MBC1_RAM: setCartFeatures(true, false); break;
			case CARTTYPE_MBC1_RAM_BATTERY: setCartFeatures(true, true); break;
		}

		loadCartData();

		if (m_ramData != nullptr){
			m_ramMapping = new ArrayMemoryMapping(m_ramData);
		} else {
			m_ramMapping = nullptr;
		}
	}

	MBC1CartMapping::~MBC1CartMapping(){

	}

	MemoryMapping* MBC1CartMapping::getRAM(){
		return m_ramMapping;
	}

	uint8_t MBC1CartMapping::get(uint16_t address){
		return m_romData[address];
	}

	void MBC1CartMapping::set(uint16_t address, uint8_t value){
		// nop
	}
}
