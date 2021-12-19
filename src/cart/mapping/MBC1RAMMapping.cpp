#include "cart/mapping/MBC1RAMMapping.hpp"


namespace toygb {
	MBC1RAMMapping::MBC1RAMMapping(uint8_t* bankSelect, bool* modeSelect, int numBanks, uint16_t bankSize, uint8_t* array, bool allowAccess) :
					FullBankedMemoryMapping(bankSelect, numBanks, bankSize, array, allowAccess){
		m_modeSelect = modeSelect;
	}

	uint8_t MBC1RAMMapping::get(uint16_t address){
		if (accessible){
			if (*m_modeSelect){
				if (*m_bankSelect > m_numBanks){
					std::stringstream errstream;
					errstream << "Out of bounds bank read : bank " << int(*m_bankSelect) << " / " << m_numBanks;
					throw EmulationError(errstream.str());
				}
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
				if (*m_bankSelect > m_numBanks){
					std::stringstream errstream;
					errstream << "Out of bounds bank write : bank " << int(*m_bankSelect) << " / " << m_numBanks;
					throw EmulationError(errstream.str());
				}
				m_array[address + (*m_bankSelect) * m_bankSize] = value;
			} else {
				m_array[address] = value;
			}
		}
	}
}
