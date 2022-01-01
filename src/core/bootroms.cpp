#include "core/bootroms.hpp"


namespace toygb {
	// Templates for builds with built-in boot ROMs

	static const bool _BOOT_DMG0_BUILTIN = false;//$$BOOT_DMG0_BUILTIN$$
	static const int _BOOT_DMG0_SIZE = 0;//$$BOOT_DMG0_SIZE$$
	static const uint8_t _BOOT_DMG0[] = {
		0//$$BOOT_DMG0$$
	};

	static const bool _BOOT_DMG_BUILTIN = false;//$$BOOT_DMG_BUILTIN$$
	static const int _BOOT_DMG_SIZE = 0;//$$BOOT_DMG_SIZE$$
	static const uint8_t _BOOT_DMG[] = {
		0//$$BOOT_DMG$$
	};

	static const bool _BOOT_MGB_BUILTIN = false;//$$BOOT_MGB_BUILTIN$$
	static const int _BOOT_MGB_SIZE = 0;//$$BOOT_MGB_SIZE$$
	static const uint8_t _BOOT_MGB[] = {
		0//$$BOOT_MGB$$
	};

	static const bool _BOOT_CGB0_BUILTIN = false;//$$BOOT_CGB0_BUILTIN$$
	static const int _BOOT_CGB0_SIZE = 0;//$$BOOT_CGB0_SIZE$$
	static const uint8_t _BOOT_CGB0[] = {
		0//$$BOOT_CGB0$$
	};

	static const bool _BOOT_CGB_BUILTIN = false;//$$BOOT_CGB_BUILTIN$$
	static const int _BOOT_CGB_SIZE = 0;//$$BOOT_CGB_SIZE$$
	static const uint8_t _BOOT_CGB[] = {
		0//$$BOOT_CGB$$
	};

	static const bool _BOOT_AGB_BUILTIN = false;//$$BOOT_AGB_BUILTIN$$
	static const int _BOOT_AGB_SIZE = 0;//$$BOOT_AGB_SIZE$$
	static const uint8_t _BOOT_AGB[] = {
		0//$$BOOT_AGB$$
	};

	static const bool _BOOT_SGB_BUILTIN = false;//$$BOOT_SGB_BUILTIN$$
	static const int _BOOT_SGB_SIZE = 0;//$$BOOT_SGB_SIZE$$
	static const uint8_t _BOOT_SGB[] = {
		0//$$BOOT_SGB$$
	};

	static const bool _BOOT_SGB2_BUILTIN = false;//$$BOOT_SGB2_BUILTIN$$
	static const int _BOOT_SGB2_SIZE = 0;//$$BOOT_SGB2_SIZE$$
	static const uint8_t _BOOT_SGB2[] = {
		0//$$BOOT_SGB2$$
	};


	bootrom_t getBootrom(GameboyConfig const& config, HardwareConfig* hardware) {
		bootrom_t bootrom = {.builtin = false, .bootrom = nullptr, .size = -1};
		const uint8_t* content = nullptr;
		std::string filename = "";
		switch (hardware->console()) {
			case ConsoleModel::DMG:
				if (hardware->system() == SystemRevision::DMG_0) {
					if (_BOOT_DMG0_BUILTIN) {
						bootrom.builtin = true;
						content = _BOOT_DMG0;
						bootrom.size = _BOOT_DMG0_SIZE;
					} else {
						filename = config.defaultBootDMG0;
					}
				} else {
					if (_BOOT_DMG_BUILTIN) {
						bootrom.builtin = true;
						content = _BOOT_DMG;
						bootrom.size = _BOOT_DMG_SIZE;
					} else {
						filename = config.defaultBootDMG;
					}
				}
				break;

			case ConsoleModel::CGB:
				if (hardware->system() == SystemRevision::CGB_0) {
					if (_BOOT_CGB0_BUILTIN) {
						bootrom.builtin = true;
						content = _BOOT_CGB0;
						bootrom.size = _BOOT_CGB0_SIZE;
					} else {
						filename = config.defaultBootCGB0;
					}
				} else {
					if (_BOOT_CGB_BUILTIN) {
						bootrom.builtin = true;
						content = _BOOT_CGB;
						bootrom.size = _BOOT_CGB_SIZE;
					} else {
						filename = config.defaultBootCGB;
					}
				}
				break;

			case ConsoleModel::MGB:
				if (_BOOT_MGB_BUILTIN) {
					bootrom.builtin = true;
					content = _BOOT_MGB;
					bootrom.size = _BOOT_MGB_SIZE;
				} else {
					filename = config.defaultBootMGB;
				}
				break;

			case ConsoleModel::AGB:
			case ConsoleModel::AGS:
			case ConsoleModel::GBP:
				if (_BOOT_AGB_BUILTIN) {
					bootrom.builtin = true;
					content = _BOOT_AGB;
					bootrom.size = _BOOT_AGB_SIZE;
				} else {
					filename = config.defaultBootAGB;
				}
				break;

			case ConsoleModel::SGB:
				if (_BOOT_SGB_BUILTIN) {
					bootrom.builtin = true;
					content = _BOOT_SGB;
					bootrom.size = _BOOT_SGB_SIZE;
				} else {
					filename = config.defaultBootSGB;
				}
				break;

			case ConsoleModel::SGB2:
				if (_BOOT_SGB2_BUILTIN) {
					bootrom.builtin = true;
					content = _BOOT_SGB2;
					bootrom.size = _BOOT_SGB2_SIZE;
				} else {
					filename = config.defaultBootSGB2;
				}
				break;

			case ConsoleModel::Auto:
				throw std::logic_error("ConsoleModel::Auto given to getBootrom");
		}

		if (bootrom.builtin) {
			std::cout << "Loaded the built-in bootstrap ROM for " << std::to_string(hardware->console()) << ", revision " << std::to_string(hardware->system()) << std::endl;
			bootrom.bootrom = new uint8_t[bootrom.size];
			for (int i = 0; i < bootrom.size; i++)
				bootrom.bootrom[i] = content[i];
			return bootrom;
		}
		// Read the bootrom from a file
		else {
			std::cout << "No built-in bootstrap ROM for " << std::to_string(hardware->console()) << ", revision " << std::to_string(hardware->system()) << std::endl;
			return getBootrom(filename);
		}
	}

	// Load the bootrom from a file
	bootrom_t getBootrom(std::string filename) {
		std::cout << "Loading the bootstrap ROM from " << filename << std::endl;
		bootrom_t bootrom = {.builtin = false, .bootrom = nullptr, .size = -1};

		// Checking file existence
		std::filesystem::path filepath(filename);
		if (!std::filesystem::exists(filepath)) {
			std::cerr << "Bootrom file " << filename << " not found, defaulting to no-bootrom, check your installation and configuration" << std::endl;
			return bootrom;
		}

		std::ifstream bootromFile(filename, std::ifstream::in | std::ifstream::binary);
		if (!std::filesystem::exists(filepath)) {
			std::cerr << "Bootrom file " << filename << " could not be opened, check your installation and configuration" << std::endl;
			return bootrom;
		}

		// All this to get the file size
		bootrom.size = std::filesystem::file_size(filepath);
		bootrom.bootrom = new uint8_t[bootrom.size];
		bootromFile.read(reinterpret_cast<char*>(bootrom.bootrom), bootrom.size);
		bootromFile.close();

		return bootrom;
	}
}
