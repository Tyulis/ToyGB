#include <string>
#include <iostream>
#include <stdexcept>
#include <exception>
#include <cstring>

#include "Gameboy.hpp"
#include "GameboyConfig.hpp"
#include "core/OperationMode.hpp"
#include "util/error.hpp"


int main(int argc, char** argv){
	if (argc < 2) {
		std::cout << "Usage : " << argv[0] << " romfile <ramfile> [--disassemble]" << std::endl;
		return 1;
	}

	toygb::GameboyConfig config;
	config.romfile = std::string(argv[1]);
	if (argc >= 3) config.ramfile = std::string(argv[2]);
	if (argc >= 4){
		for (int i = 3; i < argc; i++){
			if (std::strcmp(argv[i], "--disassemble") == 0)
				config.disassemble = true;
		}
	}

	try {
		toygb::Gameboy gameboy(config);
		gameboy.init(toygb::OperationMode::Auto);
		gameboy.main();
		return 0;
	} catch (std::exception& err){
		std::cerr << err.what();
		return 1;
	}
}
