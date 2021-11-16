#include "memory/mapping/FullBankedMemoryMapping.hpp"


namespace toygb {
	FullBankedMemoryMapping::FullBankedMemoryMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array, bool allowAccess) :
							 BankedMemoryMapping(bankSelect, bankSize, array, allowAccess){
	}

	uint8_t FullBankedMemoryMapping::get(uint16_t address){
		if (accessible){
			return m_array[address + (*m_bankSelect)*m_bankSize];
		} else {
			return 0xFF;  // FIXME ?
		}
	}

	void FullBankedMemoryMapping::set(uint16_t address, uint8_t value){
		if (accessible){
			m_array[address + (*m_bankSelect)*m_bankSize] = value;
		}
	}
}
