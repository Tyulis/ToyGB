#include "cart/CartController.hpp"


namespace toygb {
	CartController::CartController(){
		m_romMapping = nullptr;
		m_ramMapping = nullptr;
	}

	CartController::~CartController(){
		if (m_romMapping != nullptr) delete m_romMapping;
	}

	void CartController::init(std::string romfile, std::string ramfile){
		std::ifstream rom(romfile, std::ifstream::in | std::ifstream::binary);
		if (!rom.is_open()){
			std::stringstream errstream;
			errstream << "ROM file " << romfile << " not found";
			throw EmulationError(errstream.str());
		}

		uint8_t carttype;

		rom.seekg(0x0147);
		rom.read(reinterpret_cast<char*>(&carttype), sizeof(uint8_t));
		rom.close();

		switch (carttype) {
			case 0x00:
			case 0x08:
			case 0x09:
				m_romMapping = new ROMCartMapping(carttype, romfile, ramfile);
				break;

			case 0x01:
			case 0x02:
			case 0x03:
				m_romMapping = new MBC1CartMapping(carttype, romfile, ramfile);
				break;

			case 0x05:
			case 0x06:
				m_romMapping = new MBC2CartMapping(carttype, romfile, ramfile);
				break;

			case 0x0B:
			case 0x0C:
			case 0x0D:
				m_romMapping = new MMM01CartMapping(carttype, romfile, ramfile);
				break;

			case 0x0F:
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
				m_romMapping = new MBC3CartMapping(carttype, romfile, ramfile);
				break;

			case 0x15:
			case 0x16:
			case 0x17:
				m_romMapping = new MBC4CartMapping(carttype, romfile, ramfile);
				break;

			case 0x19:
			case 0x1A:
			case 0x1B:
			case 0x1C:
			case 0x1D:
			case 0x1E:
				m_romMapping = new MBC5CartMapping(carttype, romfile, ramfile);
				break;
		}
		m_ramMapping = m_romMapping->getRAM();
	}

	void CartController::configureMemory(MemoryMap* memory){
		memory->add(ROM0_OFFSET, ROM0_OFFSET + ROM_SIZE - 1, m_romMapping);
		if (m_ramMapping != nullptr){
			memory->add(SRAM_OFFSET, SRAM_OFFSET + SRAM_SIZE - 1, m_ramMapping);
		}
	}

	OperationMode CartController::getAutoOperationMode() {
		return m_romMapping->getAutoOperationMode();
	}
}
