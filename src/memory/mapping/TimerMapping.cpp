#include "memory/mapping/TimerMapping.hpp"

#define OFFSET_START IO_TIMER_DIVIDER
#define OFFSET_DIVIDER IO_TIMER_DIVIDER - OFFSET_START
#define OFFSET_COUNTER IO_TIMER_COUNTER - OFFSET_START
#define OFFSET_MODULO  IO_TIMER_MODULO - OFFSET_START
#define OFFSET_CONTROL IO_TIMER_CONTROL - OFFSET_START

#define HIGH_TO_LOW(prev, next, bit) (((prev >> bit) & 1) && !((next >> bit) & 1))

namespace toygb {
	const int TIMA_CYCLE_COUNTS[] = {1024, 16, 64, 256};
	const int TIMA_TRIGGER_BITS[] = {9, 3, 5, 7};
	const int DIV_TRIGGER_BIT = 7;

	TimerMapping::TimerMapping(OperationMode mode, InterruptVector* interrupt) {
		m_mode = mode;
		m_interrupt = interrupt;

		counter = 0x00;
		modulo = 0x00;
		enable = false;
		clockSelect = 0x00;

		m_internalCounter = 0;  // FIXME
		m_timaReloadDelay = 0xFF;
	}

	uint8_t TimerMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_DIVIDER: return m_internalCounter >> 8;
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
			case OFFSET_DIVIDER: resetDivider(); break;
			case OFFSET_COUNTER:
				if (m_timaReloadDelay != 0)  // FIXME
					counter = value;
				break;
			case OFFSET_MODULO: modulo = value; break;
			case OFFSET_CONTROL:
				enable = (value >> 2) & 1;
				clockSelect = value & 3;
				break;
		}
		//std::cout << oh8(value) << " " << oh8(divider) << " " << oh8(counter) << " " << oh8(modulo) << " " << enable << " " << oh8(clockSelect) << std::endl;
	}

	void TimerMapping::resetDivider(){
		checkTimerIncrements(m_internalCounter, 0);
		m_internalCounter = 0;
	}

	void TimerMapping::incrementCounter(int cycles, bool stopped){
		if (!stopped){
			for (int i = 0; i < cycles; i++){  // FIXME
				if (m_timaReloadDelay != 0xFF){
					m_timaReloadDelay -= 1;
					if (m_timaReloadDelay == 0){
						counter = modulo;
						m_timaReloadDelay = 0xFF;
					}
				}

				checkTimerIncrements(m_internalCounter, m_internalCounter + 1);
				m_internalCounter += 1;
			}
		}
	}

	void TimerMapping::checkTimerIncrements(uint16_t previousValue, uint16_t newValue){
		if (enable){
			if (HIGH_TO_LOW(previousValue, newValue, TIMA_TRIGGER_BITS[clockSelect])){
				if (counter == 0xFF){
					m_interrupt->setRequest(Interrupt::Timer);
					counter = 0;
					m_timaReloadDelay = 4;
				} else {
					counter += 1;
				}
			}
		}
	}
}
