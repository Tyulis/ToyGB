#include "memory/mapping/SerialTransferMapping.hpp"

#define OFFSET_START IO_SERIAL_DATA
#define OFFSET_DATA    IO_SERIAL_DATA - OFFSET_START
#define OFFSET_CONTROL IO_SERIAL_CONTROL - OFFSET_START

namespace toygb {
	SerialTransferMapping::SerialTransferMapping(OperationMode mode) {
		m_mode = mode;
		transferData = 0x00;
		transferStartFlag = false;
		clockSpeed = true;
		shiftClock = (mode == OperationMode::CGB);
	}

	uint8_t SerialTransferMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_DATA: return transferData;
			case OFFSET_CONTROL: return (transferStartFlag << 7) | (m_mode == OperationMode::CGB ? (clockSpeed << 1) : 1) | shiftClock | 0x7C;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	void SerialTransferMapping::set(uint16_t address, uint8_t value){
		switch (address) {
			case OFFSET_DATA: transferData = value; break;
			case OFFSET_CONTROL:
				transferStartFlag = (value >> 7) & 1;
				if (m_mode == OperationMode::CGB) clockSpeed = (value >> 1) & 1;
				shiftClock = value & 1;
				break;
		}
	}
}
