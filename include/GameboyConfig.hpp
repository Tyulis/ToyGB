#ifndef _GAMEBOYCONFIG_HPP
#define _GAMEBOYCONFIG_HPP

#include <string>

#include "core/hardware.hpp"

namespace toygb {
	/** Holds the emulator configuration */
	class GameboyConfig {
		public:
			GameboyConfig();

			std::string bootrom;  // Bootrom file path
			std::string romfile;  // ROM file path
			std::string ramfile;  // Save file path

			ConsoleModel console;   // Console to emulate, defaults to ConsoleModel::Auto to select according to the ROM header
			OperationMode mode;     // Operation mode to start into, defaults to OperationMode::Auto
			SystemRevision system;  // CPU revision to emulate, defaults to SystemRevision::Auto

			bool disassemble;
	};
}

#endif
