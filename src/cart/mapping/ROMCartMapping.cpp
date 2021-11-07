#include "cart/mapping/ROMCartMapping.hpp"


namespace toygb {
	ROMCartMapping::ROMCartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {
		if (m_sramdata != nullptr){
			m_sramMapping = new ArrayMemoryMapping(m_sramdata);
		} else {
			m_sramMapping = nullptr;
		}
	}

	ROMCartMapping::~ROMCartMapping(){

	}

	MemoryMapping* ROMCartMapping::getSRAM(){
		return m_sramMapping;
	}

	uint8_t ROMCartMapping::get(uint16_t address){
		return m_romdata[address];
	}

	void ROMCartMapping::set(uint16_t address, uint8_t value){
		// nop
	}
}
