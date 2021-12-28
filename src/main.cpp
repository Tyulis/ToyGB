#include <string>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <exception>
#include <cstring>

#include "Gameboy.hpp"
#include "GameboyConfig.hpp"
#include "core/hardware.hpp"
#include "util/error.hpp"


using namespace toygb;

// Decode the --mode argument
OperationMode argumentOperationMode(std::string value) {
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c){ return std::toupper(c); });

	if (value == "GB")         return OperationMode::DMG;
	else if (value == "DMG")   return OperationMode::DMG;
	else if (value == "GBC")   return OperationMode::CGB;
	else if (value == "CGB")   return OperationMode::CGB;
	else if (value == "COLOR") return OperationMode::CGB;
	else if (value == "AUTO")  return OperationMode::Auto;
	else throw std::runtime_error("Invalid operation mode (--mode argument)");
}

// Decode the --system argument
SystemRevision argumentSystem(std::string value) {
	value.erase(std::remove(value.begin(), value.end(), '-'), value.end());
	value.erase(std::remove(value.begin(), value.end(), '_'), value.end());

	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c){ return std::toupper(c); });

	if (value == "DMG0")       return SystemRevision::DMG_0;
	else if (value == "DMGA")  return SystemRevision::DMG_A;
	else if (value == "DMGB")  return SystemRevision::DMG_B;
	else if (value == "DMGC")  return SystemRevision::DMG_C;
	else if (value == "DMG")   return SystemRevision::DMG_C;

	else if (value == "MGB")   return SystemRevision::MGB;

	else if (value == "CGB0")  return SystemRevision::CGB_0;
	else if (value == "CGBA")  return SystemRevision::CGB_A;
	else if (value == "CGBB")  return SystemRevision::CGB_B;
	else if (value == "CGBC")  return SystemRevision::CGB_C;
	else if (value == "CGBD")  return SystemRevision::CGB_D;
	else if (value == "CGBE")  return SystemRevision::CGB_E;
	else if (value == "CGB")   return SystemRevision::CGB_E;

	else if (value == "AGB0")  return SystemRevision::AGB_0;
	else if (value == "AGBA")  return SystemRevision::AGB_A;
	else if (value == "AGBAE") return SystemRevision::AGB_AE;
	else if (value == "AGBB")  return SystemRevision::AGB_B;
	else if (value == "AGBBE") return SystemRevision::AGB_BE;
	else if (value == "AGB")   return SystemRevision::AGB_A;
	else if (value == "GBP")   return SystemRevision::AGB_A;

	else if (value == "SGB")   return SystemRevision::SGB;
	else if (value == "SGB2")  return SystemRevision::SGB2;

	else throw std::runtime_error("Invalid hardware platform (--system argument)");
}

// Decode the --console argument
ConsoleModel argumentConsole(std::string value) {
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c){ return std::toupper(c); });

	if (value == "GB")        return ConsoleModel::DMG;
	else if (value == "DMG")  return ConsoleModel::DMG;
	else if (value == "MGB")  return ConsoleModel::MGB;
	else if (value == "GBC")  return ConsoleModel::CGB;
	else if (value == "CGB")  return ConsoleModel::CGB;
	else if (value == "SGB")  return ConsoleModel::SGB;
	else if (value == "SGB2") return ConsoleModel::SGB2;
	else if (value == "GBA")  return ConsoleModel::AGB;
	else if (value == "AGB")  return ConsoleModel::AGB;
	else if (value == "GBS")  return ConsoleModel::AGS;
	else if (value == "AGS")  return ConsoleModel::AGB;
	else if (value == "GBP")  return ConsoleModel::GBP;
	else if (value == "AUTO") return ConsoleModel::Auto;
	else throw std::runtime_error("Invalid console model (--console argument)");
}

void printHelp(char* argv0) {
	std::cout << "Usage : " << argv0 << " romfile [arguments]" << std::endl << std::endl;
	std::cout << "Emulation options : " << std::endl;
	std::cout << "--save=<save file>  : Manually set save file" << std::endl;
	std::cout << "--mode=<mode>       : Force operation mode" << std::endl;
	std::cout << "\tValues  : DMG, CGB, auto" << std::endl;
	std::cout << "\tAliases : GB = DMG, GBC = CGB, color = CGB" << std::endl;
	std::cout << "--console=<model>   : Force console model to emulate" << std::endl;
	std::cout << "\tValues  : DMG, CGB, MGB, CGB, AGB, AGS, SGB, SGB2, GBP, auto" << std::endl;
	std::cout << "\tAliases : GB = DMG, GBC = CGB, GBA = AGB, GBS = AGS" << std::endl;
	std::cout << "--system=<revision> : Force SoC revision to emulate" << std::endl;
	std::cout << "\tValues  : DMG0, DMG-A, DMG-B, DMG-C, MGB" << std::endl;
	std::cout << "\t          CGB0, CGB-A, CGB-B, CGB-C, CGB-D, CGB-E" << std::endl;
	std::cout << "\t          AGB0, AGB-A, AGB-AE, AGB-B, AGB-BE" << std::endl;
	std::cout << "\t          SGB, SGB2" << std::endl;
	std::cout << "\tAliases : DMG = DMG-C, CGB = CGB-E, AGB = AGB-A, GBP = AGB-A" << std::endl;
	std::cout << std::endl << "Debug options : " << std::endl;
	std::cout << "--disassemble : Print disassembly to console" << std::endl;

}

int main(int argc, char** argv) {
	if (argc < 2) {
		printHelp(argv[0]);
		return 1;
	} else if (std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0){
		printHelp(argv[0]);
		return 0;
	}

	GameboyConfig config;
	config.romfile = std::string(argv[1]);
	config.ramfile = config.romfile.substr(0, config.romfile.find_last_of('.')) + ".sav";

	if (argc >= 3) {
		for (int i = 2; i < argc; i++) {
			std::string argument = std::string(argv[i]);
			std::string key = "", value = "";

			if (argument.find_last_of('=') == std::string::npos) {
			 	key = argument;
			} else {
				key = argument.substr(0, argument.find_last_of('='));
				value = argument.substr(argument.find_last_of('='));
			}

			if (key == "--disassemble"){
				config.disassemble = true;
			} else if (key == "--mode"){
				config.mode = argumentOperationMode(value);
			} else if (key == "--system"){
				config.system = argumentSystem(value);
			} else if (key == "--console"){
				config.console = argumentConsole(value);
			} else if (key == "--save"){
				config.ramfile = value;
			}
		}
	}

	try {
		Gameboy gameboy(config);
		gameboy.main();
		return 0;
	} catch (std::exception& err){
		std::cerr << err.what();
		return 2;
	}
}
