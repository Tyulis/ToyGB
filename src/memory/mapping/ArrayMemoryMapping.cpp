#include "memory/mapping/ArrayMemoryMapping.hpp"


namespace toygb {
	// Initialize the memory mapping
	ArrayMemoryMapping::ArrayMemoryMapping(uint8_t* array) {
		m_array = array;
	}

	// Get the value at the given relative address
	uint8_t ArrayMemoryMapping::get(uint16_t address) {
		return m_array[address];
	}

	// Set the value at the given relative address
	void ArrayMemoryMapping::set(uint16_t address, uint8_t value) {
		m_array[address] = value;
	}
}
