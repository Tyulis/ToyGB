#ifndef _CORE_TIMING_HPP
#define _CORE_TIMING_HPP

// Clock (base clock ticks) frequency in Hz (=4194304)
#define CLOCK_FREQUENCY 0x400000

// Machine (CPU) cycle frequency in Hz : 1 CPU cycle = 4 clocks
#define MACHINE_FREQUENCY (CLOCK_FREQUENCY/4)

// Clock period in nanoseconds (truncated)
#define CLOCK_CYCLE_NS (1000000000 / CLOCK_FREQUENCY)

// Clock period in nanoseconds (more precise)
#define CLOCK_CYCLE_NS_REAL (1000000000.0 / CLOCK_FREQUENCY)

// CPU cycle period in nanoseconds
#define MACHINE_CYCLE_NS (1000000000 / MACHINE_FREQUENCY)

#include <chrono>

namespace toygb {
	typedef std::chrono::time_point<std::chrono::steady_clock> clocktime_t;
}

#endif
