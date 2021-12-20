#include "memory/mapping/OAMDMAMapping.hpp"


namespace toygb {
	OAMDMAMapping::OAMDMAMapping(HardwareConfig& hardware){
		m_hardware = hardware;
		sourceAddress = hardware.isCGBCapable() ? 0x00 : 0xFF;
		active = false;
	}

	uint8_t OAMDMAMapping::get(uint16_t address) {
		return sourceAddress >> 8;
	}

	void OAMDMAMapping::set(uint16_t address, uint8_t value){
		sourceAddress = (uint16_t)value << 8;
		active = true;
	}
}
