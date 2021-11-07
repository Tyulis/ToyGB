#include "util/error.hpp"


namespace toygb {
	static std::string _buildEmulationErrorMessage(std::string message, std::string file, std::string function, int line){
		std::stringstream ss;
		ss << "EmulationError, in " << file << ":" << function << ", line " << line << " : " << message;
		return ss.str().c_str();
	}

	_EmulationError::_EmulationError(std::string message, std::string file, std::string function, int line) :
		std::runtime_error(_buildEmulationErrorMessage(message, file, function, line)){

	}
}
