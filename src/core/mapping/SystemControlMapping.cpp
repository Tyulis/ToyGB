#include "core/mapping/SystemControlMapping.hpp"

#define OFFSET_START IO_KEY0
#define OFFSET_KEY0 IO_KEY0 - OFFSET_START
#define OFFSET_KEY1 IO_KEY1 - OFFSET_START

/** Large-scale CGB hardware control IO registers

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF4C |       0000 | KEY0 | ----WW-- | Compatibility mode control
           |            |      |          | - 0b00 : Normal CGB mode
           |            |      |          | - 0b01 : DMG compatibility mode
           |            |      |          | - 0b10 : PGB1 mode, CPU stopped, LCD controlled externally
           |            |      |          | - 0b11 : PGB2 mode, CPU operating, LCD controlled externally
      FF4D |       0001 | KEY1 | R------B | Double-speed mode control : S------P
           |            |      |          | - S (bit 7) : Returns the current speed mode (0 = normal, 1 = double-speed)
           |            |      |          | - P (bit 0) : Prepare a speed switch (1 = prepare, 0 = don't)
           |            |      |          |               If this bit is set to 1, the speed mode will change when a stop opcode is executed */



namespace toygb {
	// Initialize the memory mapping
	SystemControlMapping::SystemControlMapping(HardwareConfig* hardware) {
		m_hardware = hardware;
		m_prepareSpeedSwitch = false;
	}

	// Get the value at the given relative address
	uint8_t SystemControlMapping::get(uint16_t address) {
		if (m_hardware->isCGBCapable()) {
			switch (address) {
				case OFFSET_KEY0:
					return 0xFF;
				case OFFSET_KEY1:
					return (m_hardware->doubleSpeed() << 7) | m_prepareSpeedSwitch | 0x7E;
			}
			std::stringstream errstream;
			errstream << "Wrong memory mapping : " << oh16(address);
			throw EmulationError(errstream.str());
		} else {
			return 0xFF;
		}
	}

	// Set the value at the given relative address
	void SystemControlMapping::set(uint16_t address, uint8_t value) {
		if (m_hardware->isCGBCapable()) {
			switch (address) {
				case OFFSET_KEY0:
					if (!m_hardware->bootromUnmapped()) {
						uint8_t mode = (value >> 2) & 0b11;
						if (mode == 0)
							m_hardware->setOperationMode(OperationMode::CGB);
						else if (mode == 1)
							m_hardware->setOperationMode(OperationMode::DMG);
						else
							std::cerr << "Tried to set KEY0 to a PGB mode, not implemented (" << oh8(mode) << ")" << std::endl;
					}
					break;
				case OFFSET_KEY1:
					m_prepareSpeedSwitch = value & 1;
					break;
				default:
					std::stringstream errstream;
					errstream << "Wrong memory mapping : " << oh16(address);
					throw EmulationError(errstream.str());
			}
		}
	}
}
