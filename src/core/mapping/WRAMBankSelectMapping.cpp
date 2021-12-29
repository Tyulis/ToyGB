#include "core/mapping/WRAMBankSelectMapping.hpp"

/** WRAM bank selector, CGB-mode only IO register

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF70 |       0000 | SVBK | -----BBB | Select the WRAM bank to map in address range 0xD000-0xDFFF
           |            |      |          | Selecting bank 0 will map bank 1 instead */



namespace toygb {
	// Initialize the memory mapping
	WRAMBankSelectMapping::WRAMBankSelectMapping(uint8_t* reg){
		m_register = reg;
	}

	// Get the value at the given relative address
	uint8_t WRAMBankSelectMapping::get(uint16_t address){
		return *m_register;
	}

	// Set the value at the given relative address
	void WRAMBankSelectMapping::set(uint16_t address, uint8_t value){
		*m_register = value & 7;
	}
}
