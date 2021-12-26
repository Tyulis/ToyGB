#ifndef _CORE_MAPPING_INTERRUPTREGISTERMAPPING_HPP
#define _CORE_MAPPING_INTERRUPTREGISTERMAPPING_HPP

#include "memory/MemoryMapping.hpp"


namespace toygb {
	/** Memory mapping for an interrupt status register (IE / IF) */
	class InterruptRegisterMapping : public MemoryMapping {
		public:
			/** Initialize the mapping, `holdUpperBits` tells whether the upper, unused bits must be conserved (they can hold any value, but are unused by the hardware), or not kept */
			InterruptRegisterMapping(bool holdUpperBits);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			bool interrupts[5];  // Index in this array is the integer value of the Interrupt

		private:
			bool m_holdUpperBits;  // Whether to hold the upper, unused bits value
			uint8_t m_upperBits;   // Upper bits value
	};
}

#endif
