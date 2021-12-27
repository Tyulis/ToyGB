#include "graphics/mapping/LCDMemoryMapping.hpp"


namespace toygb {
	// Initialize the memory mapping
	LCDMemoryMapping::LCDMemoryMapping(uint8_t* array) {
		accessible = true;
		m_array = array;
	}

	// Get the value at the given memory address (CPU access, unavailable if reserved by the PPU)
	uint8_t LCDMemoryMapping::get(uint16_t address) {
		if (!accessible) return 0xFF;
		return m_array[address];
	}

	// Set the value at the given memory address (CPU access, unavailable if reserved by the PPU)
	void LCDMemoryMapping::set(uint16_t address, uint8_t value) {
		if (accessible)
			m_array[address] = value;
	}

	// Get the value at the given memory address (PPU access, unavailable if not reserved)
	uint8_t LCDMemoryMapping::lcdGet(uint16_t address) {
		if (accessible) return 0xFF;
		return m_array[address];
	}

	// Set the value at the given memory address (PPU access, unavailable if not reserved)
	void LCDMemoryMapping::lcdSet(uint16_t address, uint8_t value) {
		if (!accessible)
			m_array[address] = value;
	}
}
