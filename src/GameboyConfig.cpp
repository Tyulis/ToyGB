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
		defaultBootCGB0 = "boot/toyboot_cgb0.bin";
		defaultBootCGB = "boot/toyboot_cgb.bin";
		defaultBootAGB = "boot/toyboot_agb.bin";
		defaultBootSGB = "boot/toyboot_sgb.bin";
		defaultBootSGB2 = "boot/toyboot_sgb2.bin";
	}
}
