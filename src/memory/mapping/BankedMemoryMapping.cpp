#include "memory/mapping/BankedMemoryMapping.hpp"


namespace toygb {
	BankedMemoryMapping::BankedMemoryMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array, bool allowAccess) {
		m_bankSelect = bankSelect;
		m_bankSize = bankSize;
		m_array = array;
		accessible = allowAccess;
	}
}
