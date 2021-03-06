#include "cart/CartController.hpp"


namespace toygb {
	// Initialize with null values, the actual initialization is in CartController::init
	CartController::CartController() {
		m_romMapping = nullptr;
		m_ramMapping = nullptr;
	}

	CartController::~CartController() {
		if (m_romMapping != nullptr) delete m_romMapping;
		m_romMapping = nullptr;
	}

	// Initialize the cartridge with the given ROM and save file names
	void CartController::init(std::string romfile, std::string ramfile, HardwareStatus* hardware) {
		m_romfile = romfile;
		m_ramfile = ramfile;
		m_hardware = hardware;

		// Open the ROM file
		std::ifstream rom(romfile, std::ifstream::in | std::ifstream::binary);
		if (!rom.is_open()) {
			std::stringstream errstream;
			errstream << "ROM file " << romfile << " not found";
			throw EmulationError(errstream.str());
		}

		// Read the cartridge cart identifier at 0x0147 in the ROM header
		uint8_t carttype;

		rom.seekg(0x0147);
		rom.read(reinterpret_cast<char*>(&carttype), sizeof(uint8_t));
		rom.close();

		// Identify the cartridge type
		switch (carttype) {
			case 0x00: case 0x08: case 0x09:
				m_romMapping = new ROMCartMapping(carttype, romfile, ramfile, hardware);
				break;

			case 0x01: case 0x02: case 0x03:
				m_romMapping = new MBC1CartMapping(carttype, romfile, ramfile, hardware);
				break;

			case 0x05: case 0x06:
				m_romMapping = new MBC2CartMapping(carttype, romfile, ramfile, hardware);
				break;

			case 0x0B: case 0x0C: case 0x0D:
				m_romMapping = new MMM01CartMapping(carttype, romfile, ramfile, hardware);
				break;

			case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13:
				m_romMapping = new MBC3CartMapping(carttype, romfile, ramfile, hardware);
				break;

			case 0x15: case 0x16: case 0x17:
				m_romMapping = new MBC4CartMapping(carttype, romfile, ramfile, hardware);
				break;
			case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
				m_romMapping = new MBC5CartMapping(carttype, romfile, ramfile, hardware);
				break;

			default:
				std::cerr << "Unknown cart type found at 0x0147 in ROM header : " << oh8(carttype) << ", defaulting to no-MBC ROM" << std::endl;
				m_romMapping = new ROMCartMapping(carttype, romfile, ramfile, hardware);
				break;
		}

		// Get the RAM mapping, if any
		m_ramMapping = m_romMapping->getRAM();
	}

	// Configure the memory mappings
	void CartController::configureMemory(MemoryMap* memory) {
		memory->add(ROM0_OFFSET, ROM0_OFFSET + ROM_SIZE - 1, m_romMapping);
		if (m_ramMapping != nullptr)
			memory->add(SRAM_OFFSET, SRAM_OFFSET + SRAM_SIZE - 1, m_ramMapping);
	}

	// Get a default hardware configuration from the ROM
	HardwareStatus CartController::getDefaultHardwareStatus() const {
		return m_romMapping->getDefaultHardwareStatus();
	}

	// Save the cartridge RAM to the save file if necessary
	void CartController::save() {
		if (m_romMapping->hasBattery() && !m_ramfile.empty()){
			std::ofstream savefile(m_ramfile, std::ofstream::out | std::ofstream::binary);
			m_ramMapping->save(savefile);
			savefile.close();
		}
	}

	// Tell whether the cartridge has integrated RAM
	bool CartController::hasRAM() const {
		return m_romMapping->hasRAM();
	}

	// Tell whether the cartridge includes a battery (= saves its RAM)
	bool CartController::hasBattery() const {
		return m_romMapping->hasBattery();
	}

	// Tell whether the cartridge includes a Real-Time Clock
	bool CartController::hasRTC() const {
		return m_romMapping->hasRTC();
	}

	// Update the cartridge status
	void CartController::update() {
		m_romMapping->update();
	}
}
