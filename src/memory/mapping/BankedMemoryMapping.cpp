#include "memory/mapping/BankedMemoryMapping.hpp"


namespace toygb {
	BankedMemoryMapping::BankedMemoryMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array, bool allowAccess) {
		m_bankSelect = bankSelect;
		m_bankSize = bankSize;
		m_array = array;
		accessible = allowAccess;
	}

	uint8_t BankedMemoryMapping::get(uint16_t address){
		if (accessible){
			if (address < m_bankSize || *m_bankSelect <= 1){
				return m_array[address];
			} else {
				return m_array[address + ((*m_bankSelect - 1) * m_bankSize)];
			}
		} else {
			return 0xFF;  // FIXME ?
		}
	}

	void BankedMemoryMapping::set(uint16_t address, uint8_t value){
		if (accessible){
			if (address < m_bankSize || *m_bankSelect <= 1){
				m_array[address] = value;
			} else {
				m_array[address + ((*m_bankSelect - 1) * m_bankSize)] = value;
			}
		}
	}
}
