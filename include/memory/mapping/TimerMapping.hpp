#ifndef _MEMORY_MAPPING_TIMERMAPPING_HPP
#define _MEMORY_MAPPING_TIMERMAPPING_HPP

#include "core/OperationMode.hpp"
#include "core/InterruptVector.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class TimerMapping : public MemoryMapping {
		public:
			TimerMapping(OperationMode mode, InterruptVector* interrupt);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			void resetDivider();
			void incrementCounter(int cycles, bool stopped);

			uint8_t divider;
			uint8_t counter;
			uint8_t modulo;
			bool enable;
			uint8_t clockSelect;

		private:
			OperationMode m_mode;
			InterruptVector* m_interrupt;

			uint64_t m_internalCounter;
	};
}

#endif
