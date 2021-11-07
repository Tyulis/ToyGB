#include "control/JoypadController.hpp"


namespace toygb {
	JoypadController::JoypadController() {
		m_interrupt = nullptr;
		m_register = nullptr;
	}

	JoypadController::~JoypadController() {
		if (m_register != nullptr) delete m_register;
	}

	void JoypadController::init(OperationMode mode, InterruptVector* interrupt){
		m_mode = mode;
		m_interrupt = interrupt;
		m_register = new JoypadMapping();
	}

	void JoypadController::configureMemory(MemoryMap* memory){
		memory->add(IO_JOYPAD, IO_JOYPAD, m_register);
	}
}
