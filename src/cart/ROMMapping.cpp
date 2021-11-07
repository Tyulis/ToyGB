#include "cart/ROMMapping.hpp"


namespace toygb {
	ROMMapping::ROMMapping(uint8_t carttype, std::string romfile, std::string ramfile) {
		m_carttype = carttype;
		m_romfile = romfile;
		m_sramfile = ramfile;

		std::ifstream rom(romfile, std::ifstream::in | std::ifstream::binary);
		rom.seekg(0, std::ifstream::end);
		int romsize = rom.tellg();
		m_romdata = new uint8_t[romsize];
		rom.seekg(0);
		rom.read(reinterpret_cast<char*>(m_romdata), romsize);
		rom.close();

		if (ramfile != ""){
			std::ifstream sram(ramfile, std::ifstream::in | std::ifstream::binary);
			sram.seekg(0, std::ifstream::end);
			int sramsize = sram.tellg();
			m_sramdata = new uint8_t[sramsize];
			sram.seekg(0);
			sram.read(reinterpret_cast<char*>(m_sramdata), sramsize);
			sram.close();
		} else {
			m_sramdata = nullptr;
		}
	}

	ROMMapping::~ROMMapping(){
		if (m_romdata != nullptr) delete[] m_romdata;
		if (m_sramdata != nullptr) delete[] m_sramdata;
	}

	OperationMode ROMMapping::getAutoOperationMode(){
		return OperationMode::DMG;
		/* if (m_romdata[0x0143] & 0x80){
			return OperationMode::CGB;
		} else {
			return OperationMode::DMG;
		}*/
	}
}
