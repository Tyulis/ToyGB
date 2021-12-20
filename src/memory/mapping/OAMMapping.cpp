#include "memory/mapping/OAMMapping.hpp"

#define OFFSET_START OAM_OFFSET
#define OFFSET_UNUSED OAM_END_OFFSET - OFFSET_START
#define OFFSET_FEC0 0x00C0

namespace toygb {
	OAMMapping::OAMMapping(HardwareConfig& hardware, uint8_t* array) : LCDMemoryMapping(array){
		m_hardware = hardware;
		m_fea0 = nullptr;
		m_fec0 = nullptr;

		if (hardware.isCGBConsole()){
			// From The Cycle-Accurate Gameboy Documentation, by Antonio Niño Díaz (AntonioND)
			// Only verified on revision D, probably variable between revisions
			m_fea0 = new uint8_t[32];
			for (int i = 0; i < 32; i++) m_fea0[i] = 0;
			m_fec0 = new uint8_t[16];
			for (int i = 0; i < 16; i++) m_fec0[i] = 0;
		}
	}

	OAMMapping::~OAMMapping(){
		if (m_fea0 == nullptr) delete m_fea0;
		if (m_fec0 == nullptr) delete m_fec0;
		m_fea0 = nullptr;
		m_fec0 = nullptr;
	}

	uint8_t OAMMapping::get(uint16_t address){
		if (!accessible) return 0xFF;  // TODO : OAM corruption in mode 2

		if (address < OFFSET_UNUSED){
			return m_array[address];
		} else {  // Unused 0xFEA0-0xFEFF area
			// FIXME : Not verified on SGB ?
			if (m_hardware.isDMGConsole() || m_hardware.isSGBConsole()){
				return 0x00;
			} else if (m_hardware.isCGBConsole() && m_hardware.system() != SystemRevision::CGB_E){  // CGB revision <D
				if (address < OFFSET_FEC0) return m_fea0[address - OFFSET_UNUSED];
				else return m_fec0[(address - OFFSET_FEC0) % 16];
			} else if (m_hardware.isAGBConsole() || (m_hardware.isCGBConsole() && m_hardware.system() == SystemRevision::CGB_E)){  // AGB and CGB-E
				uint8_t nibble = (address >> 4) & 0x0F;
				return nibble | (nibble << 4);
			} else {
				return 0x00;
			}
		}
	}

	void OAMMapping::set(uint16_t address, uint8_t value){
		if (accessible){
			if (address < OFFSET_UNUSED){
				m_array[address] = value;
			} else if (m_hardware.isCGBConsole() && m_hardware.system() != SystemRevision::CGB_E){  // CGB revision <D
				if (address < OFFSET_FEC0) m_fea0[address - OFFSET_UNUSED] = value;
				else m_fec0[(address - OFFSET_FEC0) % 16] = value;
			}
		}
	}

	uint8_t OAMMapping::lcdGet(uint16_t address){
		if (accessible) return 0xFF;
		return m_array[address];
	}

	void OAMMapping::lcdSet(uint16_t address, uint8_t value){
		if (!accessible)
			m_array[address] = value;
	}
}
