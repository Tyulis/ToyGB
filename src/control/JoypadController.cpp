#include "control/JoypadController.hpp"


namespace toygb {
	// Initialize the component with null values (actual initialization is in JoypadController::init)
	JoypadController::JoypadController() {
		m_interrupt = nullptr;
		m_register = nullptr;
	}

	JoypadController::~JoypadController() {
		if (m_register != nullptr) delete m_register;
		m_register = nullptr;
	}

	// Initialize the component, with the interrupt register as it controls the joypad interrupt (TODO : currently not implemented)
	void JoypadController::init(HardwareStatus* hardware, InterruptVector* interrupt) {
		m_hardware = hardware;
		m_interrupt = interrupt;
		m_register = new JoypadMapping();
	}

	// Configure the associated memory mappings
	void JoypadController::configureMemory(MemoryMap* memory) {
		memory->add(IO_JOYPAD, IO_JOYPAD, m_register);
	}

	// Set the given button’s status (handled by the JOYP register mapping)
	void JoypadController::setButton(JoypadButton button, bool pressed) {
		m_register->setButton(button, pressed);
	}
}
