#include "communication/mapping/InfraredTransferMapping.hpp"

/** Infrared communication IO registers mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF56 |       0000 |   RP | BB----RB | Infrared LED and receiver interface : EE----RW
           |            |      |          | - E (bits 6-7) : Enable reading (3 = read, 0 = don’t)
           |            |      |          | - R (bit 1) : Bit read by the infrared receiver
           |            |      |          | - W (bit 0) : Bit to send (0 = LED off, 1 = LED on) */

namespace toygb {
	// Initialize the memory mapping
	InfraredTransferMapping::InfraredTransferMapping(HardwareStatus* hardware) {
		m_hardware = hardware;

		// Default register values (RP = 0xFF)
		dataWrite = dataRead = 1;
		dataReadEnable = 3;
	}

	// Get the value at the given relative address
	uint8_t InfraredTransferMapping::get(uint16_t address) {
		return (dataReadEnable << 6) | (dataRead << 1) | dataWrite;
	}

	// Set the value at the given relative address
	void InfraredTransferMapping::set(uint16_t address, uint8_t value) {
		dataReadEnable = (value >> 6) & 3;
		dataWrite = value & 1;
	}
}
