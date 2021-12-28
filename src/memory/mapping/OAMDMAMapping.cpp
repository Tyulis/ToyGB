#include "memory/mapping/OAMDMAMapping.hpp"

/** OAM DMA register mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF46 |       0000 |  DMA | BBBBBBBB | Contains the upper byte of the source address for OAM DMA
           |            |      |          | When 0xNN is written to this registers, starts the transfer of bytes 0xNN00-0xNNA0 to 0xFE00-0xFEA0 */


namespace toygb {
	// Initialize the memory mapping with its initial values
	OAMDMAMapping::OAMDMAMapping(HardwareConfig& hardware) {
		m_hardware = hardware;
		sourceAddress = hardware.isCGBCapable() ? 0x00 : 0xFF;
		active = false;
	}

	// Get the value at the given relative address
	uint8_t OAMDMAMapping::get(uint16_t address) {
		return sourceAddress >> 8;  // Always goes from 0xXX00 to 0xXXA0, the upper byte is always constant
	}

	// Set the value at the given relative address
	void OAMDMAMapping::set(uint16_t address, uint8_t value) {
		sourceAddress = (uint16_t)value << 8;
		active = true;
	}
}
