#include "memory/mapping/InterruptRegisterMapping.hpp"


namespace toygb {
	InterruptRegisterMapping::InterruptRegisterMapping(bool holdUpperBits){
		m_holdUpperBits = holdUpperBits;
		m_upperBits = 0x00;

		for (int i = 0; i < 5; i++){
			interrupts[i] = false;
		}
	}

	uint8_t InterruptRegisterMapping::get(uint16_t address){
		uint8_t result = 0x00;
		for (int i = 0; i < 5; i++){
			result |= interrupts[i] << i;
		}

		// IE can retain any value in its unused bits (The Cycle-Accurate Game Boy Docs, by Antonio Niño Díaz, section 2.5)
		if (m_holdUpperBits)
			result |= m_upperBits;
		return result;
	}

	void InterruptRegisterMapping::set(uint16_t address, uint8_t value){
		for (int i = 0; i < 5; i++){
			interrupts[i] = (value >> i) & 1;
		}
		if (m_holdUpperBits)
			m_upperBits = value & 0b11100000;
	}
}
