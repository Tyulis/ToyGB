#include "graphics/mapping/LCDBankedMemoryMapping.hpp"


namespace toygb {
	// Initialize the memory mapping
	LCDBankedMemoryMapping::LCDBankedMemoryMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array) : LCDMemoryMapping(array) {
		accessible = true;
		m_bankSelect = bankSelect;
		m_bankSize = bankSize;
	}

	// Get the value at the given memory address (CPU access, unavailable if reserved by the PPU)
	uint8_t LCDBankedMemoryMapping::get(uint16_t address) {
		if (!accessible) return 0xFF;
		// The area is fully switchable
		return m_array[address + (*m_bankSelect * m_bankSize)];
	}

	// Set the value at the given memory address (CPU access, unavailable if reserved by the PPU)
	void LCDBankedMemoryMapping::set(uint16_t address, uint8_t value) {
		if (accessible)
			m_array[address + (*m_bankSelect * m_bankSize)] = value;
	}

	// Get the value at the given memory address (PPU access, unavailable if not reserved)
	uint8_t LCDBankedMemoryMapping::lcdGet(uint16_t address) {
		if (accessible) return 0xFF;
		return m_array[address + (*m_bankSelect * m_bankSize)];
	}

	// Set the value at the given memory address (PPU access, unavailable if reserved by the PPU)
	void LCDBankedMemoryMapping::lcdSet(uint16_t address, uint8_t value) {
		if (!accessible)
			m_array[address + (*m_bankSelect * m_bankSize)] = value;
	}
}
