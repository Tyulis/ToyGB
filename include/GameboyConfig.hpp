#ifndef _GAMEBOYCONFIG_HPP
#define _GAMEBOYCONFIG_HPP

#include <string>

namespace toygb {
	class GameboyConfig {
		public:
			GameboyConfig();

			std::string romfile;
			std::string ramfile;
			bool disassemble;
	};
}

#endif
