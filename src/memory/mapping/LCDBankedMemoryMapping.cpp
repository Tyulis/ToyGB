#include "memory/mapping/LCDBankedMemoryMapping.hpp"


namespace toygb {
	LCDBankedMemoryMapping::LCDBankedMemoryMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array) : LCDMemoryMapping(array){
		accessible = true;
		m_bankSelect = bankSelect;
		m_bankSize = bankSize;
	}

	uint8_t LCDBankedMemoryMapping::get(uint16_t address){
		if (!accessible) return 0xFF;
		return m_array[address + (*m_bankSelect * m_bankSize)];
	}

	void LCDBankedMemoryMapping::set(uint16_t address, uint8_t value){
		if (accessible)
			m_array[address + (*m_bankSelect * m_bankSize)] = value;
	}

	uint8_t LCDBankedMemoryMapping::lcdGet(uint16_t address){
		if (accessible) return 0xFF;
		return m_array[address + (*m_bankSelect * m_bankSize)];
	}

	void LCDBankedMemoryMapping::lcdSet(uint16_t address, uint8_t value){
		if (!accessible) 
			m_array[address + (*m_bankSelect * m_bankSize)] = value;
	}
}
