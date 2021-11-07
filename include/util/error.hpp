#ifndef _UTIL_ERROR_HPP
#define _UTIL_ERROR_HPP

#include <ios>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#define oh16(value) std::hex << std::setfill('0') << std::setw(4) << int(value) << std::dec << std::setfill(' ')
#define oh8(value) std::hex << std::setfill('0') << std::setw(2) << int(value) << std::dec << std::setfill(' ')
#define EmulationError(message) _EmulationError(message, __FILE__, __FUNCTION__, __LINE__)


namespace toygb {
	class _EmulationError : public std::runtime_error {
		public:
			_EmulationError(std::string message, std::string file, std::string function, int line);
	};
}

#endif
