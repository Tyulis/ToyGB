#include "cart/mapping/MBC5CartMapping.hpp"


namespace toygb {
	MBC5CartMapping::MBC5CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {

	}

	MBC5CartMapping::~MBC5CartMapping(){

	}

	MemoryMapping* MBC5CartMapping::getSRAM(){
		return m_sramMapping;
	}

	uint8_t MBC5CartMapping::get(uint16_t address){
		return m_romdata[address];
	}

	void MBC5CartMapping::set(uint16_t address, uint8_t value){
		// nop
	}
}
