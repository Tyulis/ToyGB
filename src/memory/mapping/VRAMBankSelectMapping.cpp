#include "memory/mapping/VRAMBankSelectMapping.hpp"


namespace toygb {
	VRAMBankSelectMapping::VRAMBankSelectMapping(uint8_t* reg){
		m_register = reg;
	}

	uint8_t VRAMBankSelectMapping::get(uint16_t address){
		return *m_register;
	}

	void VRAMBankSelectMapping::set(uint16_t address, uint8_t value){
		*m_register = value & 1;
	}
}
