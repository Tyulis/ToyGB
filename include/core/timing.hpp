#ifndef _CORE_TIMING_HPP
#define _CORE_TIMING_HPP

// Clock (base clock ticks) frequency in Hz (=4194304)
#define CLOCK_FREQUENCY 0x400000
#define DOUBLESPEED_CLOCK_FREQUENCY (CLOCK_FREQUENCY*2)

// Machine (CPU) cycle frequency in Hz : 1 CPU cycle = 4 clocks
#define MACHINE_FREQUENCY (CLOCK_FREQUENCY/4)
#define DOUBLESPEED_MACHINE_FREQUENCY (DOUBLESPEED_CLOCK_FREQUENCY/4)

// Clock period in nanoseconds (truncated)
#define CLOCK_CYCLE_NS (1000000000 / CLOCK_FREQUENCY)
#define DOUBLESPEED_CLOCK_CYCLE_NS (1000000000 / DOUBLESPEED_CLOCK_FREQUENCY)

// Clock period in nanoseconds (more precise)
#define CLOCK_CYCLE_NS_REAL (1000000000.0 / CLOCK_FREQUENCY)
#define DOUBLESPEED_CLOCK_CYCLE_NS_REAL (1000000000.0 / DOUBLESPEED_CLOCK_FREQUENCY)

// CPU cycle period in nanoseconds
#define MACHINE_CYCLE_NS (1000000000 / MACHINE_FREQUENCY)
#define DOUBLESPEED_MACHINE_CYCLE_NS (1000000000 / DOUBLESPEED_MACHINE_FREQUENCY)

// Number of clock cycles to run at once
#define BLOCK_CYCLES 400
// Minimum time to realign the clock timing by
#define MIN_WAIT_TIME_NS (BLOCK_CYCLES*CLOCK_CYCLE_NS)

#include <chrono>

namespace toygb {
	typedef std::chrono::time_point<std::chrono::steady_clock> clocktime_t;
}

#endif
