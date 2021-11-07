#include "cart/mapping/MMM01CartMapping.hpp"


namespace toygb {
	MMM01CartMapping::MMM01CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {

	}

	MMM01CartMapping::~MMM01CartMapping(){

	}

	MemoryMapping* MMM01CartMapping::getSRAM(){
		return m_sramMapping;
	}

	uint8_t MMM01CartMapping::get(uint16_t address){
		return m_romdata[address];
	}

	void MMM01CartMapping::set(uint16_t address, uint8_t value){
		// nop
	}
}
