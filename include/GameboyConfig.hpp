#ifndef _GAMEBOYCONFIG_HPP
#define _GAMEBOYCONFIG_HPP

#include <string>

#include "core/hardware.hpp"

namespace toygb {
	/** Holds the emulator configuration
	 *  TODO : Use a proper config file */
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

			// Default boot ROM files
			std::string defaultBootDMG0;
			std::string defaultBootDMG;
			std::string defaultBootMGB;
			std::string defaultBootCGB0;
			std::string defaultBootCGB;
			std::string defaultBootAGB;
			std::string defaultBootSGB;
			std::string defaultBootSGB2;
	};
}

#endif
