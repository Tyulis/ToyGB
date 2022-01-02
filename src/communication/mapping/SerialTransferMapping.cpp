#include "communication/mapping/SerialTransferMapping.hpp"

#define OFFSET_START IO_SERIAL_DATA
#define OFFSET_DATA    IO_SERIAL_DATA - OFFSET_START
#define OFFSET_CONTROL IO_SERIAL_CONTROL - OFFSET_START

/** Serial communication IO registers mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF01 |       0000 |   SB | BBBBBBBB | Value to transmit or value received through the serial port
      FF02 |       0001 |   SC | B-----BB | Serial port control register : T-----SC
           |            |      |          | - T (bit 7) : Transfer start flag (0 = idle, 1 = a transfer is in progress or requested)
           |            |      |          | - S (bit 1) : CGB mode only, clock speed (0 = normal, 1 = fast)
           |            |      |          | - C (bit 0) : Clock to use (0 = external, 1 = internal) */

namespace toygb {
	// Initialize the memory mapping
	SerialTransferMapping::SerialTransferMapping(HardwareStatus* hardware) {
		m_hardware = hardware;

		// Default register values (SB = 0x00, SC = 0x7E on non-CGB, 0x7F on CGB-enabled)
		transferData = 0x00;
		transferStartFlag = false;
		clockSpeed = true;
		shiftClock = hardware->isCGBCapable();  // Startup value is 0 on DMG models, 1 on CGB and AGB models
	}

	// Get the value at the given relative address
	uint8_t SerialTransferMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_DATA:  // SB
				return transferData;
			case OFFSET_CONTROL:  // SC
				return (transferStartFlag << 7) | ((m_hardware->mode() == OperationMode::CGB ? clockSpeed : 1) << 1) | shiftClock | 0x7C;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	// Set the value at the given relative address
	void SerialTransferMapping::set(uint16_t address, uint8_t value) {
		switch (address) {
			case OFFSET_DATA:  // SB
				transferData = value;
				break;
			case OFFSET_CONTROL:  // SC
				transferStartFlag = (value >> 7) & 1;
				if (m_hardware->mode() == OperationMode::CGB)
					clockSpeed = (value >> 1) & 1;
				shiftClock = value & 1;
				break;
		}
	}
}
