#include "communication/SerialController.hpp"


namespace toygb {
	// Initialize the component with null values (actual initialization is in SerialController::init)
	SerialController::SerialController() {
		m_mapping = nullptr;
	}

	SerialController::~SerialController() {
		if (m_mapping != nullptr) delete m_mapping;
		m_mapping = nullptr;
	}

	// Initialize the component, with the interrupt vector as it controls the
	// serial communication interrupt, that is not implemented for the moment
	void SerialController::init(HardwareConfig* hardware, InterruptVector* interrupt) {
		m_hardware = hardware;
		m_interrupt = interrupt;
		m_mapping = new SerialTransferMapping(hardware);
	}

	// Configure the associated memory mappings
	void SerialController::configureMemory(MemoryMap* memory) {
		memory->add(IO_SERIAL_DATA, IO_SERIAL_CONTROL, m_mapping);
	}
}
