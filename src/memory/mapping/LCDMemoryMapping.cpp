#include "memory/mapping/LCDMemoryMapping.hpp"


namespace toygb {
	LCDMemoryMapping::LCDMemoryMapping(uint8_t* array){
		accessible = true;
		m_array = array;
	}

	uint8_t LCDMemoryMapping::get(uint16_t address){
		if (!accessible) return 0xFF;
		return m_array[address];
	}

	void LCDMemoryMapping::set(uint16_t address, uint8_t value){
		if (accessible) {
			m_array[address] = value;
		}
	}
}
