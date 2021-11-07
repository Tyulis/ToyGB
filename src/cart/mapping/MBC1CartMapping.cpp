#include "cart/mapping/MBC1CartMapping.hpp"


namespace toygb {
	MBC1CartMapping::MBC1CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile){

	}

	MBC1CartMapping::~MBC1CartMapping(){

	}

	MemoryMapping* MBC1CartMapping::getSRAM(){
		return m_sramMapping;
	}

	uint8_t MBC1CartMapping::get(uint16_t address){
		return m_romdata[address];
	}

	void MBC1CartMapping::set(uint16_t address, uint8_t value){
		// nop
	}
}
