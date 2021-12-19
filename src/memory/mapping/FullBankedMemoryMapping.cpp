#include "memory/mapping/FullBankedMemoryMapping.hpp"


namespace toygb {
	FullBankedMemoryMapping::FullBankedMemoryMapping(uint8_t* bankSelect, int numBanks, uint16_t bankSize, uint8_t* array, bool allowAccess) :
							 BankedMemoryMapping(bankSelect, numBanks, bankSize, array, allowAccess){
	}

	uint8_t FullBankedMemoryMapping::get(uint16_t address){
		if (accessible){
			if (*m_bankSelect > m_numBanks){
				std::stringstream errstream;
				errstream << "Out of bounds bank read : bank " << int(*m_bankSelect) << " / " << m_numBanks;
				throw EmulationError(errstream.str());
			}
			return m_array[address + (*m_bankSelect)*m_bankSize];
		} else {
			return 0xFF;  // FIXME ?
		}
	}

	void FullBankedMemoryMapping::set(uint16_t address, uint8_t value){
		if (accessible){
			if (*m_bankSelect > m_numBanks){
				std::stringstream errstream;
				errstream << "Out of bounds bank write : bank " << int(*m_bankSelect) << " / " << m_numBanks;
				throw EmulationError(errstream.str());
			}
			m_array[address + (*m_bankSelect)*m_bankSize] = value;
		}
	}
}
