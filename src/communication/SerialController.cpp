#include "communication/SerialController.hpp"


namespace toygb {
	SerialController::SerialController() {
		m_mapping = nullptr;
	}

	SerialController::~SerialController() {
		if (m_mapping != nullptr) delete m_mapping;
	}

	void SerialController::init(HardwareConfig& hardware, InterruptVector* interrupt){
		m_hardware = hardware;
		m_interrupt = interrupt;
		m_mapping = new SerialTransferMapping(hardware);
	}

	void SerialController::configureMemory(MemoryMap* memory) {
		memory->add(IO_SERIAL_DATA, IO_SERIAL_CONTROL, m_mapping);
	}
}
