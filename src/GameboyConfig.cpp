#include "GameboyConfig.hpp"

namespace toygb {
	GameboyConfig::GameboyConfig(){
		romfile = "";
		ramfile = "";

		mode = OperationMode::Auto;
		system = SystemRevision::Auto;
		console = ConsoleModel::Auto;

		disassemble = false;
	}
}
