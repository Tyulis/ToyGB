#include "cart/ROMMapping.hpp"


namespace toygb {
	// Initialize the memory mapping
	ROMMapping::ROMMapping(uint8_t carttype, std::string romfile, std::string ramfile, HardwareStatus* hardware) {
		m_hardware = hardware;
		m_cartType = carttype;
		m_romFile = romfile;
		m_ramFile = ramfile;
	}

	ROMMapping::~ROMMapping() {
		if (m_romData != nullptr) delete[] m_romData;
		if (m_ramData != nullptr) delete[] m_ramData;
		m_romData = nullptr;
		m_ramData = nullptr;
	}

	// Build an automatic hardware configuration to run this ROM, based on its header
	HardwareStatus ROMMapping::getDefaultHardwareStatus() const {
		// TODO : SGB config
		return HardwareStatus(OperationMode::Auto, (m_romData[0x0143] & 0x80 ? ConsoleModel::CGB : ConsoleModel::DMG), SystemRevision::Auto);
	}

	bool ROMMapping::hasRAM() const {
		return m_hasRAM;
	}

	bool ROMMapping::hasBattery() const {
		return m_hasBattery;
	}

	bool ROMMapping::hasRTC() const {
		return m_hasRTC;
	}

	// Set cartridge feature flags based on the cartridge type identifier
	void ROMMapping::setCartFeatures(bool hasRAM, bool hasBattery, bool hasRTC) {
		m_hasRAM = hasRAM;
		m_hasBattery = hasBattery;
		m_hasRTC = hasRTC;
	}

	// Load the cartridge data from ROM, and dimension ROM and RAM data in memory
	void ROMMapping::loadCartData() {
		// Open the ROM file
		std::ifstream rom(m_romFile, std::ifstream::in | std::ifstream::binary);
		if (!rom.is_open()) {
			std::stringstream errstream;
			errstream << "ROM file " << m_romFile << " not found";
			throw EmulationError(errstream.str());
		}

		// Read 0x0148 (ROM size exponent) and 0x0149 (RAM size identifier)
		rom.seekg(0x0148);
		uint8_t sizeExponents[2];
		rom.read(reinterpret_cast<char*>(sizeExponents), 2*sizeof(uint8_t));

		m_romSize = 0x8000 << sizeExponents[0];

		// Dimension the RAM
		switch (sizeExponents[1]) {
			case 0x00: m_ramSize = 0; break;
			case 0x02: m_ramSize = 0x2000; break;
			case 0x03: m_ramSize = 0x8000; break;
			case 0x04: m_ramSize = 0x20000; break;
			case 0x05: m_ramSize = 0x10000; break;
			default: m_ramSize = 0; break;
		}

		// Read the ROM data from the file into memory
		m_romData = new uint8_t[m_romSize];
		rom.seekg(0);
		rom.read(reinterpret_cast<char*>(m_romData), m_romSize);
		rom.close();

		// Create RAM array
		if (m_hasRAM) {
			// Some homebrew ROMs expect RAM but don’t set any size, so we set it to a single bank of cartridge RAM (0x2000) by default
			// A real gameboy would not care (those values in the header were probably only for emulators used for testing anyway)
			if (m_ramSize == 0) {
				std::cerr << "Warning : cart type (" << oh8(m_cartType) << ") indicates cart RAM but with size zero, defaulting to a single bank" << std::endl;
				m_ramSize = 0x2000;
			}

			m_ramData = new uint8_t[m_ramSize];
		} else {
			m_ramData = nullptr;
		}
	}

	// Load the cartridge RAM content from the save file if it exists
	void ROMMapping::loadSaveData(MemoryMapping* ramMapping) {
		// Load RAM content from the save file if it already exists
		if (m_hasBattery) {
			std::ifstream ram(m_ramFile, std::ifstream::in | std::ifstream::binary);
			if (ram.is_open()) {
				ramMapping->load(ram);
				ram.close();
			}
		}
	}

	// Update the cartridge status, like the RTC
	void ROMMapping::update() {

	}
}
