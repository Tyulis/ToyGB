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

		defaultBootDMG0 = "boot/toyboot_dmg0.bin";
		defaultBootDMG = "boot/toyboot_dmg.bin";
		defaultBootMGB = "boot/toyboot_mgb.bin";
		
		// TODO
		defaultBootCGB0 = "boot/toyboot_dmg.bin";
		defaultBootCGB = "boot/toyboot_dmg.bin";
		defaultBootAGB = "boot/toyboot_dmg.bin";
		defaultBootSGB = "boot/toyboot_dmg.bin";
		defaultBootSGB2 = "boot/toyboot_dmg.bin";
	}
}
