#ifndef _MEMORY_MAPPING_INTERRUPTREGISTERMAPPING_HPP
#define _MEMORY_MAPPING_INTERRUPTREGISTERMAPPING_HPP

#include "memory/MemoryMapping.hpp"


namespace toygb {
	class InterruptRegisterMapping : public MemoryMapping {
		public:
			InterruptRegisterMapping(bool holdUpperBits);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			bool interrupts[5];

		private:
			bool m_holdUpperBits;
			uint8_t m_upperBits;
	};
}

#endif
