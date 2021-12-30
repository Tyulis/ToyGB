#include "core/hardware.hpp"


namespace toygb {
	// Initialize the hardware configuration with default values (Auto)
	HardwareConfig::HardwareConfig() {
		m_mode = OperationMode::Auto;
		m_console = ConsoleModel::Auto;
		m_system = SystemRevision::Auto;
		m_hasBootrom = false;
	}

	// Initialize the hardware configuration with custom values
	HardwareConfig::HardwareConfig(OperationMode mode, ConsoleModel console, SystemRevision system) {
		m_mode = mode;
		m_console = console;
		m_system = system;
		m_hasBootrom = false;
	}

	// Get the console model
	ConsoleModel HardwareConfig::console() const {
		return m_console;
	}

	// Set the console model and check that the configuration is coherent
	void HardwareConfig::setConsoleModel(ConsoleModel console) {
		checkSystemCompatibility(console, m_system);
		checkModeCompatibility(console, m_mode);
		m_console = console;
	}

	// Get the operation mode
	OperationMode HardwareConfig::mode() const {
		return m_mode;
	}

	// Set the operation mode and check that the configuration is coherent
	void HardwareConfig::setOperationMode(OperationMode mode) {
		checkModeCompatibility(m_console, mode);
		m_mode = mode;
	}

	// Get the CPU revision
	SystemRevision HardwareConfig::system() const {
		return m_system;
	}

	// Set the CPU revision and check that the configuration is coherent
	void HardwareConfig::setSystemRevision(SystemRevision system) {
		checkSystemCompatibility(m_console, system);
		m_system = system;
	}

	// Tell whether a bootrom is set up and valid
	bool HardwareConfig::hasBootrom() const {
		return m_hasBootrom;
	}

	// Set whether a bootrom is set up and valid
	void HardwareConfig::setBootrom(bool bootromPresent) {
		m_hasBootrom = bootromPresent;

		// CGB-capable models always start in CGB-mode, the bootrom sets DMG-compatibility mode itself by writing to KEY0
		if (m_hasBootrom && isCGBCapable())
			m_mode = OperationMode::CGB;
	}

	// Set the remaining auto settings
	void HardwareConfig::setAutoConfig() {
		// Nothing set : default CGB mode
		if (m_mode == OperationMode::Auto && m_console == ConsoleModel::Auto && m_system == SystemRevision::Auto) {
			m_mode = OperationMode::CGB;
			m_console = ConsoleModel::CGB;
			m_system = SystemRevision::CGB_E;
		}

		// Console set : set the operation mode and system revision accordingly
		else if (m_mode == OperationMode::Auto && m_system == SystemRevision::Auto) {
			m_mode = defaultOperationMode(m_console);
			m_system = defaultSystemRevision(m_console);
		}

		// Operation mode set
		else if (m_console == ConsoleModel::Auto && m_system == SystemRevision::Auto) {
			m_console = defaultConsoleModel(m_mode);
			m_system = defaultSystemRevision(m_console);
		}

		// System revision set
		else if (m_mode == OperationMode::Auto && m_console == ConsoleModel::Auto) {
			m_console = defaultConsoleModel(m_system);
			m_mode = defaultOperationMode(m_console);
		}

		// Console and system set
		else if (m_mode == OperationMode::Auto) {
			m_mode = defaultOperationMode(m_console);
		}

		// Mode and system set
		else if (m_console == ConsoleModel::Auto) {
			m_console = defaultConsoleModel(m_system);
			checkModeCompatibility(m_console, m_mode);
		}

		// Mode and console set
		else if (m_system == SystemRevision::Auto) {
			m_system = defaultSystemRevision(m_console);
		}
	}

	// Tell whether the console is a DMG model (DMG / MGB)
	bool HardwareConfig::isDMGConsole() const {
		return (m_console == ConsoleModel::DMG || m_console == ConsoleModel::MGB);
	}

	// Tell whether the console is specifically a CGB model (CGB)
	bool HardwareConfig::isCGBConsole() const {
		return (m_console == ConsoleModel::CGB);
	}

	// Tell whether the console is a SGB model (SGB / SGB2)
	bool HardwareConfig::isSGBConsole() const {
		return (m_console == ConsoleModel::SGB || m_console == ConsoleModel::SGB2);
	}

	// Tell whether the console is a AGB model (AGB / AGS / GBP)
	bool HardwareConfig::isAGBConsole() const {
		return (m_console == ConsoleModel::AGB || m_console == ConsoleModel::AGS || m_console == ConsoleModel::GBP);
	}

	// Tell whether the hardware is capable of running in CGB mode (= is CGB or newer) (CGB / AGB / AGS / GBP)
	bool HardwareConfig::isCGBCapable() const {
		return (m_console == ConsoleModel::CGB || m_console == ConsoleModel::AGB ||
				m_console == ConsoleModel::AGS || m_console == ConsoleModel::GBP);
	}


	// Check whether the console and the operation mode are coherent and throw an exception if they are not
	void HardwareConfig::checkModeCompatibility(ConsoleModel console, OperationMode mode) {
		if (console == ConsoleModel::Auto || mode == OperationMode::Auto)  // Assume Auto is compatible with anything because it will be adjusted later
			return;

		bool isIncompatible = false;
		switch (mode) {
			case OperationMode::Auto: break;
			case OperationMode::DMG: break;  // All gameboy hardwares can run in DMG mode
			case OperationMode::CGB:
				if (console != ConsoleModel::CGB && console != ConsoleModel::AGB &&
					console != ConsoleModel::AGS && console != ConsoleModel::GBP)
					isIncompatible = true;
				break;
		}

		if (isIncompatible) {
			std::stringstream errstream;
			errstream << "Incompatible console and operation mode : " << std::to_string(mode) << " mode on console " << std::to_string(console);
			throw EmulationError(errstream.str());
		}
	}

	// Check whether the console and the CPU revision are coherent and throw an exception if they are not
	// A CPU revision must be associated with the console it has been designed for
	void HardwareConfig::checkSystemCompatibility(ConsoleModel console, SystemRevision system) {
		if (console == ConsoleModel::Auto || system == SystemRevision::Auto)  // Assume Auto is compatible with anything because it will be adjusted later
			return;

		bool isIncompatible = false;
		switch (console) {
			case ConsoleModel::Auto: break;
			case ConsoleModel::DMG:
				if (system != SystemRevision::DMG_0 && system != SystemRevision::DMG_A &&
					system != SystemRevision::DMG_B && system != SystemRevision::DMG_C)
					isIncompatible = true;
				break;
			case ConsoleModel::MGB:
				if (system != SystemRevision::MGB)
					isIncompatible = true;
				break;
			case ConsoleModel::CGB:
				if (system != SystemRevision::CGB_0 && system != SystemRevision::CGB_A &&
					system != SystemRevision::CGB_B && system != SystemRevision::CGB_C &&
					system != SystemRevision::CGB_D && system != SystemRevision::CGB_E)
					isIncompatible = true;
				break;
			case ConsoleModel::SGB:
				if (system != SystemRevision::SGB)
					isIncompatible = true;
				break;
			case ConsoleModel::SGB2:
				if (system != SystemRevision::SGB2)
					isIncompatible = true;
				break;
			case ConsoleModel::AGB:
				if (system != SystemRevision::AGB_0 && system != SystemRevision::AGB_A && system != SystemRevision::AGB_AE)
					isIncompatible = true;
				break;
			case ConsoleModel::AGS:
				if (system != SystemRevision::AGB_B && system != SystemRevision::AGB_BE)
					isIncompatible = true;
				break;
			case ConsoleModel::GBP:
				if (system != SystemRevision::AGB_A && system != SystemRevision::AGB_AE)
					isIncompatible = true;
				break;
		}

		if (isIncompatible) {
			std::stringstream errstream;
			errstream << "Incompatible CPU and console : " << std::to_string(system) << " CPU on console " << std::to_string(console);
			throw EmulationError(errstream.str());
		}
	}

	// Get a default operation mode for the given console
	OperationMode HardwareConfig::defaultOperationMode(ConsoleModel console) {
		if (m_console == ConsoleModel::CGB || m_console == ConsoleModel::AGB || m_console == ConsoleModel::AGS || m_console == ConsoleModel::GBP)
			return OperationMode::CGB;
		else
			return OperationMode::DMG;
	}

	// Get a default console for the given operation mode
	ConsoleModel HardwareConfig::defaultConsoleModel(OperationMode mode) {
		if (mode == OperationMode::CGB)
			return ConsoleModel::CGB;
		else
			return ConsoleModel::DMG;
	}

	// Get the console associated to the given SoC revision
	ConsoleModel HardwareConfig::defaultConsoleModel(SystemRevision system) {
		switch (system) {
			case SystemRevision::DMG_0:
			case SystemRevision::DMG_A:
			case SystemRevision::DMG_B:
			case SystemRevision::DMG_C:
				return ConsoleModel::DMG;
			case SystemRevision::MGB:
				return ConsoleModel::MGB;
			case SystemRevision::CGB_0:
			case SystemRevision::CGB_A:
			case SystemRevision::CGB_B:
			case SystemRevision::CGB_C:
			case SystemRevision::CGB_D:
			case SystemRevision::CGB_E:
				return ConsoleModel::CGB;
			case SystemRevision::AGB_0:
			case SystemRevision::AGB_A:
			case SystemRevision::AGB_AE:
				return ConsoleModel::AGB;
			case SystemRevision::AGB_B:
			case SystemRevision::AGB_BE:
				return ConsoleModel::AGS;
			case SystemRevision::SGB:
				return ConsoleModel::SGB;
			case SystemRevision::SGB2:
				return ConsoleModel::SGB2;
			default:
				std::stringstream errstream;
				errstream << "Can not determine a default console from system revision " << std::to_string(system);
				throw EmulationError(errstream.str());
		}
	}

	// Get a default system revision for the given console
	SystemRevision HardwareConfig::defaultSystemRevision(ConsoleModel console) {
		switch (console) {
			case ConsoleModel::DMG: return SystemRevision::DMG_C;
			case ConsoleModel::MGB: return SystemRevision::MGB;
			case ConsoleModel::CGB: return SystemRevision::CGB_E;
			case ConsoleModel::AGB: return SystemRevision::AGB_A;
			case ConsoleModel::AGS: return SystemRevision::AGB_B;
			case ConsoleModel::GBP: return SystemRevision::AGB_A;
			case ConsoleModel::SGB: return SystemRevision::SGB;
			case ConsoleModel::SGB2: return SystemRevision::SGB2;
			default:
				std::stringstream errstream;
				errstream << "Can not determine a default system revision from console " << std::to_string(console);
				throw EmulationError(errstream.str());
		}
	}
}


// Convenience functions to print enum values out
namespace std {
	std::string to_string(toygb::OperationMode value) {
		switch (value) {
			case toygb::OperationMode::DMG: return "OperationMode::DMG";
			case toygb::OperationMode::CGB: return "OperationMode::CGB";
			case toygb::OperationMode::Auto: return "OperationMode::Auto";
		}
		return "";
	}

	std::string to_string(toygb::ConsoleModel value) {
		switch (value) {
			case toygb::ConsoleModel::DMG: return  "ConsoleModel::DMG";
			case toygb::ConsoleModel::MGB: return  "ConsoleModel::MGB";
			case toygb::ConsoleModel::SGB: return  "ConsoleModel::SGB";
			case toygb::ConsoleModel::SGB2: return "ConsoleModel::SGB2";
			case toygb::ConsoleModel::CGB: return  "ConsoleModel::CGB";
			case toygb::ConsoleModel::AGB: return  "ConsoleModel::AGB";
			case toygb::ConsoleModel::AGS: return  "ConsoleModel::AGS";
			case toygb::ConsoleModel::GBP: return  "ConsoleModel::GBP";
			case toygb::ConsoleModel::Auto: return "ConsoleModel::Auto";
		}
		return "";
	}

	std::string to_string(toygb::SystemRevision value) {
		switch (value) {
			case toygb::SystemRevision::DMG_0: return "SystemRevision::DMG_0";
			case toygb::SystemRevision::DMG_A: return "SystemRevision::DMG_A";
			case toygb::SystemRevision::DMG_B: return "SystemRevision::DMG_B";
			case toygb::SystemRevision::DMG_C: return "SystemRevision::DMG_C";
			case toygb::SystemRevision::MGB: return "SystemRevision::MGB";
			case toygb::SystemRevision::CGB_0: return "SystemRevision::CGB_0";
			case toygb::SystemRevision::CGB_A: return "SystemRevision::CGB_A";
			case toygb::SystemRevision::CGB_B: return "SystemRevision::CGB_B";
			case toygb::SystemRevision::CGB_C: return "SystemRevision::CGB_C";
			case toygb::SystemRevision::CGB_D: return "SystemRevision::CGB_D";
			case toygb::SystemRevision::CGB_E: return "SystemRevision::CGB_E";
			case toygb::SystemRevision::AGB_0: return "SystemRevision::AGB_0";
			case toygb::SystemRevision::AGB_A: return "SystemRevision::AGB_A";
			case toygb::SystemRevision::AGB_AE: return "SystemRevision::AGB_AE";
			case toygb::SystemRevision::AGB_B: return "SystemRevision::AGB_B";
			case toygb::SystemRevision::AGB_BE: return "SystemRevision::AGB_BE";
			case toygb::SystemRevision::SGB: return "SystemRevision::SGB";
			case toygb::SystemRevision::SGB2: return "SystemRevision::SGB2";
			case toygb::SystemRevision::Auto: return "SystemRevision::Auto";
		}
		return "";
	}
}
