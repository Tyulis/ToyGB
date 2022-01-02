#ifndef _CORE_BOOTROMS_HPP
#define _CORE_BOOTROMS_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>

#include "GameboyConfig.hpp"
#include "core/hardware.hpp"


namespace toygb {
	typedef struct {
		bool builtin;
		uint8_t* bootrom;
		int size;  // If <0, could not load the bootrom
	} bootrom_t;

	/** Load the default bootrom, either external or built-in, for the given hardware */
	bootrom_t getBootrom(GameboyConfig const& config, HardwareStatus* hardware);

	/** Load a custom external bootrom */
	bootrom_t getBootrom(std::string filename);
}

#endif
