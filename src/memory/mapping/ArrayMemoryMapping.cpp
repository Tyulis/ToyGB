#include "memory/mapping/ArrayMemoryMapping.hpp"


namespace toygb {
	ArrayMemoryMapping::ArrayMemoryMapping(uint8_t* array){
		m_array = array;
	}

	uint8_t ArrayMemoryMapping::get(uint16_t address){
		return m_array[address];
	}

	void ArrayMemoryMapping::set(uint16_t address, uint8_t value){
		m_array[address] = value;
	}
}
