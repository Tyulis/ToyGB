#ifndef _CORE_TIMING_HPP
#define _CORE_TIMING_HPP

#define CLOCK_FREQUENCY 4194304
#define MACHINE_FREQUENCY (CLOCK_FREQUENCY/4)
#define CLOCK_CYCLE_NS (1000000000 / CLOCK_FREQUENCY)
#define CLOCK_CYCLE_NS_REAL (1000000000.0 / CLOCK_FREQUENCY)
#define MACHINE_CYCLE_NS (CLOCK_CYCLE_NS * 4)

#include <chrono>

namespace toygb {
	typedef std::chrono::time_point<std::chrono::steady_clock> clocktime_t;
}

#endif
