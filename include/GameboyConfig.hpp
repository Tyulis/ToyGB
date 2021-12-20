#ifndef _GAMEBOYCONFIG_HPP
#define _GAMEBOYCONFIG_HPP

#include <string>

#include "core/hardware.hpp"

namespace toygb {
	class GameboyConfig {
		public:
			GameboyConfig();

			std::string romfile;
			std::string ramfile;

			ConsoleModel console;
			OperationMode mode;
			SystemRevision system;

			bool disassemble;
	};
}

#endif
