#include "core/mapping/BootromDisableMapping.hpp"

/** Bootrom unmap register

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF50 |       0000 | BANK | -------B | When clear, the bootrom is mapped at 0x0000, setting it to 1 disables the bootrom
           |            |      |          | Any write is OR-ed with the current value of the register, effectively locking the register to 1 after setting it */



namespace toygb {
	// Initialize the memory mapping
	BootromDisableMapping::BootromDisableMapping(HardwareStatus* hardware) {
		m_hardware = hardware;
	}

	// Get the value at the given relative address
	uint8_t BootromDisableMapping::get(uint16_t address) {
		return m_hardware->bootromUnmapped() | 0xFE;
	}

	// Set the value at the given relative address
	void BootromDisableMapping::set(uint16_t address, uint8_t value) {
		m_hardware->setBootromStatus((value & 1) | m_hardware->bootromUnmapped());
	}
}
