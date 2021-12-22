#include "cart/mapping/MBC1RAMMapping.hpp"


namespace toygb {
	// Initialize the memory mapping
	MBC1RAMMapping::MBC1RAMMapping(uint8_t* bankSelect, bool* modeSelect, int numBanks, uint16_t bankSize, uint8_t* array, bool allowAccess) :
					FullBankedMemoryMapping(bankSelect, numBanks, bankSize, array, allowAccess){
		m_modeSelect = modeSelect;
	}

	// Get the value at the given relative address
	uint8_t MBC1RAMMapping::get(uint16_t address){
		if (accessible){
			if (*m_modeSelect){  // Mode 1 : Banked RAM mode
				return m_array[address + (*m_bankSelect) * m_bankSize];
			} else {  // Mode 0 : Fixed RAM mode
				return m_array[address];
			}
		} else {  // RAM access disabled
			return 0xFF;
		}
	}

	void MBC1RAMMapping::set(uint16_t address, uint8_t value){
		if (accessible){
			if (*m_modeSelect){  // Mode 1 : Banked RAM mode
				m_array[address + (*m_bankSelect) * m_bankSize] = value;
			} else {  // Mode 0 : Fixed RAM mode
				m_array[address] = value;
			}
		}
	}
}
