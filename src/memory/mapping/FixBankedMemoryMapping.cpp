#include "memory/mapping/FixBankedMemoryMapping.hpp"


namespace toygb {
	// Initialize the memory mapping
	FixBankedMemoryMapping::FixBankedMemoryMapping(uint8_t* bankSelect, int numBanks, uint16_t bankSize, uint8_t* array, bool allowAccess) :
	 						BankedMemoryMapping(bankSelect, numBanks, bankSize, array, allowAccess){
	}

	// Get the value at the given relative address
	uint8_t FixBankedMemoryMapping::get(uint16_t address) {
		if (accessible) {
			if (address < m_bankSize || *m_bankSelect <= 1) {  // First part : first bank fixed area
				return m_array[address];
			} else {  // Second part : switchable area
				if (*m_bankSelect > m_numBanks) {
					std::stringstream errstream;
					errstream << "Out of bounds bank read : bank " << int(*m_bankSelect) << " / " << m_numBanks;
					throw EmulationError(errstream.str());
				}
				return m_array[address - m_bankSize + (*m_bankSelect) * m_bankSize];
			}
		} else {
			return 0xFF;
		}
	}

	// Set the value at the given relative address
	void FixBankedMemoryMapping::set(uint16_t address, uint8_t value) {
		if (accessible) {
			if (address < m_bankSize || *m_bankSelect <= 1) {  // First part : first bank fixed area
				m_array[address] = value;
			} else {  // Second part : switchable area
				if (*m_bankSelect > m_numBanks) {
					std::stringstream errstream;
					errstream << "Out of bounds bank write : bank " << int(*m_bankSelect) << " / " << m_numBanks;
					throw EmulationError(errstream.str());
				}
				m_array[address - m_bankSize + (*m_bankSelect) * m_bankSize] = value;
			}
		}
	}
}
