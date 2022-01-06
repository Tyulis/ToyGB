#ifndef _CORE_OPERATIONMODE_HPP
#define _CORE_OPERATIONMODE_HPP

#include <string>

#include "core/InterruptVector.hpp"
#include "core/mapping/TimerMapping.hpp"
#include "memory/MemoryMap.hpp"
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

	/** Describes the specific hardware to emulate */
	class HardwareStatus {
		public:
			HardwareStatus();
			HardwareStatus(OperationMode mode, ConsoleModel model, SystemRevision revision);

			// Console model to emulate
			ConsoleModel console() const;
			void setConsoleModel(ConsoleModel console);  // Throws an EmulationError if the config is incoherent (like CGB mode on DMG hardware or CGB CPU on DMG)

			// Operation mode (DMG / CGB) to run the program into
			OperationMode mode() const;
			void setOperationMode(OperationMode mode);  // Throws an EmulationError if the config is incoherent (like CGB mode on DMG hardware)

			// Revision of the CPU
			SystemRevision system() const;
			void setSystemRevision(SystemRevision revision);  // Throws an EmulationError if the config is incoherent (like a CGB CPU on a DMG)

			// Bootrom presence
			bool hasBootrom() const;
			void setBootrom(bool bootromPresent);

			// Bootrom mapping status
			bool bootromUnmapped() const;
			void setBootromStatus(bool unmapped);

			// Speed mode
			bool doubleSpeed() const;
			void setDoubleSpeedMode(bool isDoubleSpeed);

			// STOP mode status
			bool isStopped() const;
			void setStopMode(bool stop);

			// Trigger a speed switch
			void triggerSpeedSwitch();

			// Set the remaining Auto settings
			void setAutoConfig();

			// Component initialization and configuration
			void init(InterruptVector* interrupts);
			void configureMemory(MemoryMap* memory);
			void update();

			// Return the emulator components sequence counter value
			uint16_t getSequencer() const;

			// Divider internal counter
			uint16_t getDivider() const;
			void incrementDivider();
			void resetDivider();

			// Generic console model checks
			bool isDMGConsole() const;  // DMG, MGB
			bool isCGBConsole() const;  // CGB
			bool isAGBConsole() const;  // AGB, AGS, GBP
			bool isSGBConsole() const;  // SGB, SGB2
			bool isCGBCapable() const;  // CGB, AGB, AGS, GBP


		private:
			void checkModeCompatibility(ConsoleModel console, OperationMode mode);
			void checkSystemCompatibility(ConsoleModel console, SystemRevision system);

			OperationMode defaultOperationMode(ConsoleModel model);
			ConsoleModel defaultConsoleModel(OperationMode mode);
			ConsoleModel defaultConsoleModel(SystemRevision system);
			SystemRevision defaultSystemRevision(ConsoleModel console);

			ConsoleModel m_console;
			OperationMode m_mode;
			SystemRevision m_system;
			bool m_hasBootrom;
			bool m_bootromUnmapped;
			bool m_doubleSpeed;
			bool m_stopped;
			int m_speedSwitchCountdown;

			// Clock status
			uint16_t m_sequencer;     // Emulator components sequencer, to clock the components at the right time (CPU, audio)
			uint16_t m_divider;  // Gameboy internal divider, used for timer IO and audio frame sequencer
			TimerMapping* m_timerMapping;
	};
}

// Convenience functions to print the enum values out
namespace std {
	std::string to_string(toygb::OperationMode value);
	std::string to_string(toygb::ConsoleModel value);
	std::string to_string(toygb::SystemRevision value);
}

#endif
