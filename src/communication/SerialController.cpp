#include "communication/SerialController.hpp"


namespace toygb {
	SerialController::SerialController() {
		m_mapping = nullptr;
	}

	SerialController::~SerialController() {
		if (m_mapping != nullptr) delete m_mapping;
	}

	void SerialController::init(OperationMode mode, InterruptVector* interrupt){
		m_mode = mode;
		m_interrupt = interrupt;
		m_mapping = new SerialTransferMapping(mode);
	}

	void SerialController::configureMemory(MemoryMap* memory) {
		memory->add(IO_SERIAL_DATA, IO_SERIAL_CONTROL, m_mapping);
	}
}
