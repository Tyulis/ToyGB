#include "core/hardware.hpp"


namespace toygb {
	HardwareConfig::HardwareConfig() {
		m_mode = OperationMode::Auto;
		m_console = ConsoleModel::Auto;
		m_system = SystemRevision::Auto;
	}

	HardwareConfig::HardwareConfig(OperationMode mode, ConsoleModel console, SystemRevision system) {
		m_mode = mode;
		m_console = console;
		m_system = system;
	}

	ConsoleModel HardwareConfig::console() const {
		return m_console;
	}

	void HardwareConfig::setConsoleModel(ConsoleModel console) {
		checkSystemCompatibility(console, m_system);
		checkModeCompatibility(console, m_mode);
		m_console = console;
	}

	OperationMode HardwareConfig::mode() const {
		return m_mode;
	}

	void HardwareConfig::setOperationMode(OperationMode mode) {
		checkModeCompatibility(m_console, mode);
		m_mode = mode;
	}

	SystemRevision HardwareConfig::system() const {
		return m_system;
	}

	void HardwareConfig::setSystemRevision(SystemRevision system) {
		checkSystemCompatibility(m_console, system);
		m_system = system;
	}

	bool HardwareConfig::isDMGConsole() const {
		return (m_console == ConsoleModel::DMG || m_console == ConsoleModel::MGB);
	}

	bool HardwareConfig::isCGBConsole() const {
		return (m_console == ConsoleModel::CGB);
	}

	bool HardwareConfig::isSGBConsole() const {
		return (m_console == ConsoleModel::SGB || m_console == ConsoleModel::SGB);
	}

	bool HardwareConfig::isAGBConsole() const {
		return (m_console == ConsoleModel::AGB || m_console == ConsoleModel::AGS || m_console == ConsoleModel::GBP);
	}

	bool HardwareConfig::isCGBCapable() const {
		return (m_console == ConsoleModel::CGB || m_console == ConsoleModel::AGB ||
				m_console == ConsoleModel::AGS || m_console == ConsoleModel::GBP);
	}


	void HardwareConfig::checkModeCompatibility(ConsoleModel console, OperationMode mode) {
		if (console == ConsoleModel::Auto || mode == OperationMode::Auto)
			return;

		bool isIncompatible = false;
		switch (mode) {
			case OperationMode::Auto: break;
			case OperationMode::DMG: break;
			case OperationMode::CGB:
				if (console != ConsoleModel::CGB && console != ConsoleModel::AGB &&
					console != ConsoleModel::AGS && console != ConsoleModel::GBP)
					isIncompatible = true;
				break;
		}

		if (isIncompatible){
			std::stringstream errstream;
			errstream << "Incompatible console and operation mode : " << std::to_string(mode) << " mode on console " << std::to_string(console);
			throw EmulationError(errstream.str());
		}
	}

	void HardwareConfig::checkSystemCompatibility(ConsoleModel console, SystemRevision system) {
		if (console == ConsoleModel::Auto || system == SystemRevision::Auto)
			return;

		bool isIncompatible = false;
		switch (console){
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

		if (isIncompatible){
			std::stringstream errstream;
			errstream << "Incompatible CPU and console : " << std::to_string(system) << " CPU on console " << std::to_string(console);
			throw EmulationError(errstream.str());
		}
	}

}

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
