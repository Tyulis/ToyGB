#include "memory/mapping/FixBankedMemoryMapping.hpp"


namespace toygb {
	FixBankedMemoryMapping::FixBankedMemoryMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array, bool allowAccess) :
	 						BankedMemoryMapping(bankSelect, bankSize, array, allowAccess){
	}

	uint8_t FixBankedMemoryMapping::get(uint16_t address){
		if (accessible){
			if (address < m_bankSize || *m_bankSelect <= 1){
				return m_array[address];
			} else {
				return m_array[address + (*m_bankSelect - 1) * m_bankSize];
			}
		} else {
			return 0xFF;  // FIXME ?
		}
	}

	void FixBankedMemoryMapping::set(uint16_t address, uint8_t value){
		if (accessible){
			if (address < m_bankSize || *m_bankSelect <= 1){
				m_array[address] = value;
			} else {
				m_array[address + (*m_bankSelect - 1) * m_bankSize] = value;
			}
		}
	}
}
