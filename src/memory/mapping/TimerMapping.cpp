#include "memory/mapping/TimerMapping.hpp"

#define OFFSET_START IO_TIMER_DIVIDER
#define OFFSET_DIVIDER IO_TIMER_DIVIDER - OFFSET_START
#define OFFSET_COUNTER IO_TIMER_COUNTER - OFFSET_START
#define OFFSET_MODULO  IO_TIMER_MODULO - OFFSET_START
#define OFFSET_CONTROL IO_TIMER_CONTROL - OFFSET_START

namespace toygb {
	const int TIMA_CYCLE_COUNTS[] = {1024, 16, 64, 256};

	TimerMapping::TimerMapping(OperationMode mode, InterruptVector* interrupt) {
		m_mode = mode;
		m_interrupt = interrupt;

		divider = 0x00;  // FIXME
		counter = 0x00;
		modulo = 0x00;
		enable = false;
		clockSelect = 0x00;
	}

	uint8_t TimerMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_DIVIDER: return divider;
			case OFFSET_COUNTER: return counter;
			case OFFSET_MODULO: return modulo;
			case OFFSET_CONTROL: return (enable << 2) | clockSelect;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	void TimerMapping::set(uint16_t address, uint8_t value) {
		switch (address) {
			case OFFSET_DIVIDER: divider = 0x00; break;
			case OFFSET_COUNTER: counter = value; break;
			case OFFSET_MODULO: modulo = value; break;
			case OFFSET_CONTROL:
				enable = (value >> 2) & 1;
				clockSelect = value & 3;
				break;
		}
	}

	void TimerMapping::resetDivider(){
		divider = 0x00;
		m_dividerCounter = 0;
	}

	void TimerMapping::incrementCounter(int cycles, bool stopped){
		if (!stopped){
			m_dividerCounter += cycles;

			if (m_dividerCounter >= 0x100){
				m_dividerCounter %= 0x100;
				divider += 1;
			}
		}

		if (enable){
			m_timaCounter += cycles;
			if (m_timaCounter >= TIMA_CYCLE_COUNTS[clockSelect]){
				m_timaCounter %= TIMA_CYCLE_COUNTS[clockSelect];
				if (counter == 0xFF){
					m_interrupt->setRequest(Interrupt::Timer);
					counter = modulo;
				} else {
					counter += 1;
				}
			}
		}
	}
}
