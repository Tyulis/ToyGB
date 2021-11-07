#include "cart/mapping/MBC3CartMapping.hpp"


namespace toygb {
	MBC3CartMapping::MBC3CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {

	}

	MBC3CartMapping::~MBC3CartMapping(){

	}

	MemoryMapping* MBC3CartMapping::getSRAM(){
		return m_sramMapping;
	}

	uint8_t MBC3CartMapping::get(uint16_t address){
		return m_romdata[address];
	}

	void MBC3CartMapping::set(uint16_t address, uint8_t value){
		// nop
	}
}
