#include "memory/mapping/WRAMBankSelectMapping.hpp"


namespace toygb {
	WRAMBankSelectMapping::WRAMBankSelectMapping(uint8_t* reg){
		m_register = reg;
	}

	uint8_t WRAMBankSelectMapping::get(uint16_t address){
		return *m_register;
	}

	void WRAMBankSelectMapping::set(uint16_t address, uint8_t value){
		*m_register = value & 7;
	}
}
