#include "control/JoypadController.hpp"


namespace toygb {
	JoypadController::JoypadController() {
		m_interrupt = nullptr;
		m_register = nullptr;
	}

	JoypadController::~JoypadController() {
		if (m_register != nullptr) delete m_register;
	}

	void JoypadController::init(HardwareConfig& hardware, InterruptVector* interrupt){
		m_hardware = hardware;
		m_interrupt = interrupt;
		m_register = new JoypadMapping();
	}

	void JoypadController::configureMemory(MemoryMap* memory){
		memory->add(IO_JOYPAD, IO_JOYPAD, m_register);
	}

	void JoypadController::setButton(JoypadButton button, bool pressed){
		m_register->setButton(button, pressed);
	}
}
