#include "GameboyConfig.hpp"

namespace toygb {
	// Initial, default values for the config
	GameboyConfig::GameboyConfig() {
		bootrom = "";
		romfile = "";
		ramfile = "";

		mode = OperationMode::Auto;
		system = SystemRevision::Auto;
		console = ConsoleModel::Auto;

		disassemble = false;
	}
}
