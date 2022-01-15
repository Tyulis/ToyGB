#include "communication/CommunicationController.hpp"


namespace toygb {
	// Initialize the component with null values (actual initialization is in CommunicationController::init)
	CommunicationController::CommunicationController() {
		m_serialMapping = nullptr;
		m_infraredMapping = nullptr;
	}

	CommunicationController::~CommunicationController() {
		if (m_serialMapping != nullptr) delete m_serialMapping;
		if (m_infraredMapping != nullptr) delete m_infraredMapping;
		m_serialMapping = nullptr;
		m_infraredMapping = nullptr;
	}

	// Initialize the component, with the interrupt vector as it controls the
	// serial communication interrupt, that is not implemented for the moment
	void CommunicationController::init(HardwareStatus* hardware, InterruptVector* interrupt) {
		m_hardware = hardware;
		m_interrupt = interrupt;
		m_serialMapping = new SerialTransferMapping(hardware);

		// The infrared port is only on CGB, the AGB and AGS don’t have one
		if (m_hardware->isCGBConsole())
			m_infraredMapping = new InfraredTransferMapping(hardware);
	}

	// Configure the associated memory mappings
	void CommunicationController::configureMemory(MemoryMap* memory) {
		memory->add(IO_SERIAL_DATA, IO_SERIAL_CONTROL, m_serialMapping);
		if (m_hardware->isCGBConsole())
			memory->add(IO_INFRARED, IO_INFRARED, m_infraredMapping);
	}
}
