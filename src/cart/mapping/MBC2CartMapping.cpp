#include "cart/mapping/MBC2CartMapping.hpp"


namespace toygb {
	MBC2CartMapping::MBC2CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {

	}

	MBC2CartMapping::~MBC2CartMapping(){

	}

	MemoryMapping* MBC2CartMapping::getSRAM(){
		return m_sramMapping;
	}

	uint8_t MBC2CartMapping::get(uint16_t address){
		return m_romdata[address];
	}

	void MBC2CartMapping::set(uint16_t address, uint8_t value){
		// nop
	}
}
