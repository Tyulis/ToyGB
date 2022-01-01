#include "core/mapping/InterruptRegisterMapping.hpp"

/** Interrupt status IO register memory mapping, used for two IO registers :
(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Name | Access   | Content
      FF0F |   IF | ---BBBBB | Interrupt request register, bits associated with interrupts are set to request an interrupt : ---43210
           |      |          | Bit 0 = VBlank, bit 1 = STAT, bit 2 = timer, bit 3 = serial, bit 4 = joypad
      FFFF |   IE | BBBBBBBB | Interrupt enable register, bits associated with interrupts must be set for that interrupt to be executed : ---43210
           |      |          | Bit 0 = VBlank, bit 1 = STAT, bit 2 = timer, bit 3 = serial, bit 4 = joypad
           |      |          | The upper 3 bits are not used by the hardware, but can hold a value (The Cycle-Accurate Game Boy Docs, by Antonio Niño Díaz, section 2.5) */

namespace toygb {
	// Initialize the memory mapping
	InterruptRegisterMapping::InterruptRegisterMapping(bool holdUpperBits) {
		m_holdUpperBits = holdUpperBits;

		// The register value starts at 0x00
		m_upperBits = 0x00;
		for (int i = 0; i < 5; i++) {
			interrupts[i] = false;
		}
	}

	// Get the value at the given relative address
	uint8_t InterruptRegisterMapping::get(uint16_t address) {
		uint8_t result = 0x00;
		for (int i = 0; i < 5; i++)
			result |= interrupts[i] << i;

		// IE can retain any value in its unused bits
		if (m_holdUpperBits)
			result |= m_upperBits;
		else
			result |= 0xE0;
		return result;
	}

	// Set the value at the given relative address
	void InterruptRegisterMapping::set(uint16_t address, uint8_t value){
		for (int i = 0; i < 5; i++){
			interrupts[i] = (value >> i) & 1;
		}
		if (m_holdUpperBits)  // Keep the upper bits
			m_upperBits = value & 0b11100000;
	}
}
