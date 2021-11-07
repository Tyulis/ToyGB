#include "memory/mapping/BankedWRAMMapping.hpp"


namespace toygb {
	BankedWRAMMapping::BankedWRAMMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array) {
		m_bankSelect = bankSelect;
		m_bankSize = bankSize;
		m_array = array;
	}

	uint8_t BankedWRAMMapping::get(uint16_t address){
		if (address < m_bankSize || *m_bankSelect <= 1){
			return m_array[address];
		} else {
			return m_array[address + ((*m_bankSelect - 1) * m_bankSize)];
		}
	}

	void BankedWRAMMapping::set(uint16_t address, uint8_t value){
		if (address < m_bankSize || *m_bankSelect <= 1){
			m_array[address] = value;
		} else {
			m_array[address + ((*m_bankSelect - 1) * m_bankSize)] = value;
		}
	}
}
