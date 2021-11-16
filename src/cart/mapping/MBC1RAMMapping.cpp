#include "cart/mapping/MBC1RAMMapping.hpp"


namespace toygb {
	MBC1RAMMapping::MBC1RAMMapping(uint8_t* bankSelect, bool* modeSelect, uint16_t bankSize, uint8_t* array, bool allowAccess) :
					FullBankedMemoryMapping(bankSelect, bankSize, array, allowAccess){
		m_modeSelect = modeSelect;
	}

	uint8_t MBC1RAMMapping::get(uint16_t address){
		if (accessible){
			if (*m_modeSelect){
				return m_array[address + (*m_bankSelect) * m_bankSize];
			} else {
				return m_array[address];
			}
		} else {
			return 0xFF;  // FIXME ?
		}
	}

	void MBC1RAMMapping::set(uint16_t address, uint8_t value){
		if (accessible){
			if (*m_modeSelect){
				m_array[address + (*m_bankSelect) * m_bankSize] = value;
			} else {
				m_array[address] = value;
			}
		}
	}
}
