#ifndef _CORE_OPERATIONMODE_HPP
#define _CORE_OPERATIONMODE_HPP

#include <string>

#include "util/error.hpp"


namespace toygb {
	// Software operation mode (DMG/CGB)
	enum class OperationMode {
		DMG, CGB,
		Auto, /* Set based on cartridge header */
	};

	// SoC revision to emulate
	enum class SystemRevision {
		DMG_0, DMG_A, DMG_B, DMG_C,
		MGB,
		CGB_0, CGB_A, CGB_B, CGB_C, CGB_D, CGB_E,
		AGB_0, AGB_A, AGB_AE, AGB_B, AGB_BE,
		SGB, SGB2,
		Auto,  /* Set based on operation mode */
	};

	// Console model to emulate
	enum class ConsoleModel {
		DMG, MGB, SGB, SGB2, CGB, AGB, AGS, GBP,
		Auto,  /* Set based on operation mode */
	};

	class HardwareConfig {
		public:
			HardwareConfig();
			HardwareConfig(OperationMode mode, ConsoleModel model, SystemRevision revision);

			ConsoleModel console() const;
			void setConsoleModel(ConsoleModel console);

			OperationMode mode() const;
			void setOperationMode(OperationMode mode);

			SystemRevision system() const;
			void setSystemRevision(SystemRevision revision);

			// Console model checks
			bool isDMGConsole() const;  // DMG, MGB
			bool isCGBConsole() const;  // CGB
			bool isAGBConsole() const;  // AGB, AGS, GBP
			bool isSGBConsole() const;  // SGB, SGB2
			bool isCGBCapable() const;  // CGB, AGB, AGS, GBP


		private:
			void checkModeCompatibility(ConsoleModel console, OperationMode mode);
			void checkSystemCompatibility(ConsoleModel console, SystemRevision system);

			ConsoleModel m_console;
			OperationMode m_mode;
			SystemRevision m_system;
	};
}

namespace std {
	std::string to_string(toygb::OperationMode value);
	std::string to_string(toygb::ConsoleModel value);
	std::string to_string(toygb::SystemRevision value);
}

#endif
