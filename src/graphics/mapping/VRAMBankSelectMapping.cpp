#include "graphics/mapping/VRAMBankSelectMapping.hpp"

/** VRAM bank selector, CGB-mode only IO register

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF4F |       0000 |  VBK | -------B | Select the VRAM bank to map (in the full VRAM mapping range, 0x8000-0x9FFF */


namespace toygb {
	// Initialize the memory mapping
	VRAMBankSelectMapping::VRAMBankSelectMapping(uint8_t* reg) {
		m_register = reg;
	}

	// Get the value at the given relative address
	uint8_t VRAMBankSelectMapping::get(uint16_t address) {
		return *m_register;
	}

	// Set the value at the given relative address
	void VRAMBankSelectMapping::set(uint16_t address, uint8_t value){
		*m_register = value & 1;
	}
}
