#ifndef _UTIL_ERROR_HPP
#define _UTIL_ERROR_HPP

#include <ios>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>

/** Module with basic output and error handling utilities */

// Output a 16-bits integer in hexadecimal on an output stream (usage : stream << oh16(value))
#define oh16(value) std::hex << std::setfill('0') << std::setw(4) << int(value) << std::dec << std::setfill(' ')
// Same for 8-bits integers
#define oh8(value) std::hex << std::setfill('0') << std::setw(2) << int(value) << std::dec << std::setfill(' ')

// Define an emulation error, providing its location in the code for debugging purpose
#define EmulationError(message) _EmulationError(message, __FILE__, __FUNCTION__, __LINE__)


namespace toygb {
	/** Underlying exception type for EmulationError */
	class _EmulationError : public std::runtime_error {
		public:
			_EmulationError(std::string message, std::string file, std::string function, int line);
	};
}

#endif
