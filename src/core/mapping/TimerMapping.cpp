#include "core/mapping/TimerMapping.hpp"

#define OFFSET_START IO_TIMER_DIVIDER
#define OFFSET_DIVIDER IO_TIMER_DIVIDER - OFFSET_START
#define OFFSET_COUNTER IO_TIMER_COUNTER - OFFSET_START
#define OFFSET_MODULO  IO_TIMER_MODULO - OFFSET_START
#define OFFSET_CONTROL IO_TIMER_CONTROL - OFFSET_START

// Check whether a specific bit got from 1 to 0 between two values
#define HIGH_TO_LOW(prev, next, bit) (((prev >> bit) & 1) && !((next >> bit) & 1))

/** Timer IO registers memory mapping
The timer has an internal, hidden 16-bits counter, that keeps track of the amount of clock ticks that pass (incremented every clock)
The divider (register DIV) just returns the upper 8 bits of that internal counter directly (thus making it a 16384Hz = 4194304/256Hz timer, 32768Hz in double-speed mode)
Writing to DIV resets the internal counter, including the hidden lower 8 bits
The timer counter (register TIMA) increment frequency can be configured through the control (TAC) register.
However, it is not simply incremented every X clocks : TIMA increments are commanded through the internal counter.
When a specific bit (set through that clock frequency parameter) goes from 1 to 0 in the internal counter value, TIMA gets incremented
This detail has some importance in some obscure cases — for example, resetting the divider also impacts TIMA
When TIMA overflows, it is set to 0 for 4 clocks, then gets reset to the value of TMA. If TMA is set at the exact same cycle, the old value is transferred

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF04 |       0000 |  DIV | BBBBBBBB | Divider register, directly returns the upper 8 bits of the internal counter
           |            |      |          | Writing any value in this register resets the internal counter to 0
      FF05 |       0001 | TIMA | BBBBBBBB | Timer Accumulator (i guess), increments every time the bit set through TAC goes from 1 to 0 in the internal counter
           |            |      |          | Can be written to.
      FF06 |       0002 |  TMA | BBBBBBBB | Value to reset TIMA with when it overflows
      FF07 |       0003 |  TAC | -----BBB | TIMA timer control : -----ECC
           |            |      |          | - E (bit 2) : TIMA enable (0 = disabled, 1 = enabled). The internal counter (and thus the divider) ticks anyway
           |            |      |          | - C (bits 0-1) : Clock select. For all intents and purposes, sets the frequency of TIMA increments, see above for details (here in single-speed mode, *2 for double-speed mode):
           |            |      |          |   0b00 = every 1024 clocks (4096Hz), 0b01 = every 16 clocks (262144Hz), 0b10 = every 64 clocks (65536Hz), 0b11 = every 256 clocks (16384Hz)  */


namespace toygb {
	// Number of clocks between TIMA increments */
	// const int TIMA_UPDATE_PERIODS[] = {1024, 16, 64, 256};

	// Bits that trigger a TIMA increment when they go from 1 to 0 in the internal counter (index to use in this array is TAC.0-1)
	const int TIMA_TRIGGER_BITS[] = {9, 3, 5, 7};

	// Initialize the memory mapping with initial values
	TimerMapping::TimerMapping(HardwareConfig& hardware, InterruptVector* interrupt) {
		m_hardware = hardware;
		m_interrupt = interrupt;

		counter = 0x00;
		modulo = 0x00;
		enable = false;
		clockSelect = 0x00;

		m_internalCounter = 0;  // FIXME
		m_timaReloadDelay = 0xFF;
	}

	// Get the value at the given relative address
	uint8_t TimerMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_DIVIDER:  // DIV
				return m_internalCounter >> 8;
			case OFFSET_COUNTER:  // TIMA
				return counter;
			case OFFSET_MODULO:  // TMA
				return modulo;
			case OFFSET_CONTROL:  // TAC
				return (enable << 2) | clockSelect | 0xF8;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	// Set the value at the given relative address
	void TimerMapping::set(uint16_t address, uint8_t value) {
		switch (address) {
			case OFFSET_DIVIDER:  // DIV
				resetDivider();
				break;
			case OFFSET_COUNTER:  // TIMA
				if (m_timaReloadDelay != 0)  // FIXME
					counter = value;
				break;
			case OFFSET_MODULO:  // TMA
				modulo = value;
				break;
			case OFFSET_CONTROL:  // TAC
				enable = (value >> 2) & 1;
				clockSelect = value & 3;
				break;
		}
	}

	// Reset the internal counter
	void TimerMapping::resetDivider() {
		checkTimerIncrements(m_internalCounter, 0);  // Even resetting the divider may increment TIMA
		m_internalCounter = 0;
	}

	// Increment the internal counter and execute associated actions and increments, with the amount of clocks that passed, and whether the CPU is in stop mode
	void TimerMapping::incrementCounter(int clocks, bool stopped) {
		if (!stopped) {
			for (int i = 0; i < clocks; i++) {  // FIXME
				if (m_timaReloadDelay != 0xFF) {  // We are in the delay between TIMA overflow and its reloading with TMA : advance the reload period
					m_timaReloadDelay -= 1;
					if (m_timaReloadDelay == 0) {
						counter = modulo;
						m_timaReloadDelay = 0xFF;
					}
				}

				checkTimerIncrements(m_internalCounter, m_internalCounter + 1);
				m_internalCounter += 1;
			}
		}
	}

	// Check whether to increment TIMA, and handle the TIMA overflow logic, when going from an internal counter value to another
	void TimerMapping::checkTimerIncrements(uint16_t previousValue, uint16_t newValue) {
		if (enable) {
			// Increment if the trigger bit goes from 1 to 0, regardless of whether it was an internal counter increment, reset or overflow
			if (HIGH_TO_LOW(previousValue, newValue, TIMA_TRIGGER_BITS[clockSelect])) {
				if (counter == 0xFF) {  // TIMA overflow : timer interrupt, set TIMA to 0 and go in the 4-clock delay before reloading with TMA
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
