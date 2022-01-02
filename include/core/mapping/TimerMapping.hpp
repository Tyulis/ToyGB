#ifndef _CORE_MAPPING_TIMERMAPPING_HPP
#define _CORE_MAPPING_TIMERMAPPING_HPP

#include "core/InterruptVector.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/bits.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Timer IO registers memory mapping */
	class TimerMapping : public MemoryMapping {
		public:
			TimerMapping(uint16_t* divider, InterruptVector* interrupt);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			void dividerChange(uint16_t newValue);  // Update the timer status for a change from the current value of the divider to the given one

			uint8_t counter;      // Timer counter (register TIMA)
			uint8_t modulo;       // Timer modulo (register TMA)
			bool enable;          // Enable TIMA (register TAC, bit 2)
			uint8_t clockSelect;  // Select the increment frequency of TIMA (register TAC, bits 0-1)

		private:
			InterruptVector* m_interrupt;

			uint16_t* m_divider;        // Internal time counter, that counts clock ticks
			uint8_t m_timaReloadDelay;  // When TIMA has overflown, time before it is reloaded with TMA
	};
}

#endif
