#include "cart/mapping/MBC4CartMapping.hpp"


namespace toygb {
	MBC4CartMapping::MBC4CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {

	}

	MBC4CartMapping::~MBC4CartMapping(){

	}

	MemoryMapping* MBC4CartMapping::getSRAM(){
		return m_sramMapping;
	}

	uint8_t MBC4CartMapping::get(uint16_t address){
		return m_romdata[address];
	}

	void MBC4CartMapping::set(uint16_t address, uint8_t value){
		// nop
	}
}
