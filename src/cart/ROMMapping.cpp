#include "cart/ROMMapping.hpp"


namespace toygb {
	ROMMapping::ROMMapping(uint8_t carttype, std::string romfile, std::string ramfile) {
		m_cartType = carttype;
		m_romFile = romfile;
		m_ramFile = ramfile;
	}

	ROMMapping::~ROMMapping(){
		if (m_romData != nullptr) delete[] m_romData;
		if (m_ramData != nullptr) delete[] m_ramData;
	}

	HardwareConfig ROMMapping::getDefaultHardwareConfig() {
		return HardwareConfig(OperationMode::DMG, ConsoleModel::DMG, SystemRevision::DMG_C);  // TODO
		/* if (m_romdata[0x0143] & 0x80){
			return OperationMode::CGB;
		} else {
			return OperationMode::DMG;
		}*/
	}

	bool ROMMapping::hasRAM() const {
		return m_hasRAM;
	}

	bool ROMMapping::hasBattery() const {
		return m_hasBattery;
	}

	void ROMMapping::setCartFeatures(bool hasRAM, bool hasBattery){
		m_hasRAM = hasRAM;
		m_hasBattery = hasBattery;
	}

	void ROMMapping::loadCartData(){
		std::ifstream rom(m_romFile, std::ifstream::in | std::ifstream::binary);
		if (!rom.is_open()){
			std::stringstream errstream;
			errstream << "ROM file " << m_romFile << " not found";
			throw EmulationError(errstream.str());
		}

		rom.seekg(0x0148);
		uint8_t sizeExponents[2];
		rom.read(reinterpret_cast<char*>(sizeExponents), 2*sizeof(uint8_t));

		m_romSize = 0x8000 << sizeExponents[0];
		switch (sizeExponents[1]){
			case 0x00: m_ramSize = 0; break;
			case 0x02: m_ramSize = 0x2000; break;
			case 0x03: m_ramSize = 0x8000; break;
			case 0x04: m_ramSize = 0x20000; break;
			case 0x05: m_ramSize = 0x10000; break;
			default: m_ramSize = 0; break;
		}

		m_romData = new uint8_t[m_romSize];
		rom.seekg(0);
		rom.read(reinterpret_cast<char*>(m_romData), m_romSize);
		rom.close();

		if (m_hasRAM){
			if (m_hasBattery){
				std::ifstream ram(m_ramFile, std::ifstream::in | std::ifstream::binary);
				if (ram.is_open()){
					m_ramData = new uint8_t[m_ramSize];
					ram.read(reinterpret_cast<char*>(m_ramData), m_ramSize);
					ram.close();
				} else {
					m_ramData = new uint8_t[m_ramSize];
				}
			} else {
				if (m_ramSize == 0) {
					std::cerr << "Warning : cart type (" << oh8(m_cartType) << ") indicates cart RAM but with size zero." << std::endl;
					m_ramSize = 0x2000;
				}
				m_ramData = new uint8_t[m_ramSize];
			}
		} else {
			m_ramData = nullptr;
		}
	}
}
