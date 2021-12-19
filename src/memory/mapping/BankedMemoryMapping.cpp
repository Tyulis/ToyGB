#include "memory/mapping/BankedMemoryMapping.hpp"


namespace toygb {
	BankedMemoryMapping::BankedMemoryMapping(uint8_t* bankSelect, int numBanks, uint16_t bankSize, uint8_t* array, bool allowAccess) {
		m_bankSelect = bankSelect;
		m_bankSize = bankSize;
		m_array = array;
		m_numBanks = numBanks;
		accessible = allowAccess;
	}

	void BankedMemoryMapping::load(std::istream& input){
		input.read(reinterpret_cast<char*>(m_array), m_numBanks*m_bankSize);
	}

	void BankedMemoryMapping::save(std::ostream& output){
		output.write(reinterpret_cast<char*>(m_array), m_numBanks*m_bankSize);
	}
}
