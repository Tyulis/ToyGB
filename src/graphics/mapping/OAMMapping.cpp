#include "graphics/mapping/OAMMapping.hpp"

#define OFFSET_START OAM_OFFSET
#define OFFSET_UNUSED OAM_END_OFFSET - OFFSET_START
#define OFFSET_FEC0 0x00C0
#define OFFSET_FEE0 0x00E0

/** Details about the "forbidden" unused area 0xFEA0-0xFEFF come from The Cycle-Accurate Gameboy Documentation, by Antonio Niño Díaz (AntonioND)
https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf
And from the source code of SameBoy by LIJI32
https://github.com/LIJI32/SameBoy/blob/master/Core/memory.c#L550

This is extremely revision- and hardware-dependant, so many details are probably left untested */

namespace toygb {
	// Initialize the memory mapping
	OAMMapping::OAMMapping(HardwareStatus* hardware, uint8_t* array) : LCDMemoryMapping(array){
		m_hardware = hardware;
		m_fea0 = nullptr;
		m_fec0 = nullptr;
		m_fee0 = nullptr;

		if (hardware->isCGBConsole()) {
			// On CGB revision D, there are 32 fully readable and writable bytes from 0xFEA0 to 0xFEC0,
			// then 16 readable and writable bytes that repeat starting from 0xFEC0, 0xFED0, 0xFEE0 and 0xFEF0
			if (hardware->system() == SystemRevision::CGB_D) {
				m_fea0 = new uint8_t[32];
				for (int i = 0; i < 32; i++)
					m_fea0[i] = 0;  // FIXME : Initialize to 0 ?

				m_fec0 = new uint8_t[16];
				for (int i = 0; i < 16; i++)
					m_fec0[i] = 0;
			}

			// On CGB revisions 0, B and C at least, there are 3 groups of 8 readable and writable bytes
			// The first group repeats at 0xFEA0, 0xFEA8, 0xFEB0, 0xFEB8, the second 4 times as well on 0xFEC0-0xFEDF and the last one on 0xFEE0-0xFEFF
			else if (hardware->system() != SystemRevision::CGB_E) {
				m_fea0 = new uint8_t[8];
				m_fec0 = new uint8_t[8];
				m_fee0 = new uint8_t[8];
				for (int i = 0; i < 8; i++)
					m_fea0[i] = m_fec0[i] = m_fee0[i] = 0;
			}
		}
	}

	OAMMapping::~OAMMapping() {
		if (m_fea0 == nullptr) delete m_fea0;
		if (m_fec0 == nullptr) delete m_fec0;
		if (m_fee0 == nullptr) delete m_fee0;
		m_fea0 = m_fec0 = m_fee0 = nullptr;
	}

	// Get the value at the given memory address (CPU access, unavailable if reserved by the PPU)
	uint8_t OAMMapping::get(uint16_t address) {
		if (!accessible) return 0xFF;  // TODO : OAM corruption in mode 2

		// Normal write from OAM
		if (address < OFFSET_UNUSED) {
			return m_array[address];
		}

		// Unused 0xFEA0-0xFEFF area
		else {
			// DMG : This area returns zeros
			// FIXME : Not verified on SGB ?
			if (m_hardware->isDMGConsole() || m_hardware->isSGBConsole()){
				return 0x00;
			}

			// CGB revision D : 32-bytes area at 0xFEA0 and repeating 16 bytes area at 0xFEC0
			else if (m_hardware->isCGBConsole() && m_hardware->system() == SystemRevision::CGB_D) {
				if (address < OFFSET_FEC0)
					return m_fea0[address - OFFSET_UNUSED];
				else
					return m_fec0[(address - OFFSET_FEC0) & 0x0F];
			}

			// CGB revisions < D : Three 8 bytes groups repeating from 0xFEA0, 0xFEC0 and 0xFEE0
			else if (m_hardware->isCGBConsole() && m_hardware->system() != SystemRevision::CGB_E) {  // CGB 0-C
				if (address < OFFSET_FEC0)
					return m_fea0[(address - OFFSET_UNUSED) & 7];
				else if (address < OFFSET_FEE0)
					return m_fec0[(address - OFFSET_FEC0) & 7];
				else
					return m_fee0[(address - OFFSET_FEE0) & 7];
			}

			// On CGB revision E and AGB/AGS/GBP, this area returns the upper nibble of the address twice (like 0xFEB1 -> 0xBB)
			else if (m_hardware->isAGBConsole() || (m_hardware->isCGBConsole() && m_hardware->system() == SystemRevision::CGB_E)) {
				uint8_t nibble = (address >> 4) & 0x0F;
				return nibble | (nibble << 4);
			}

			// Other untested behaviours
			else {
				return 0x00;
			}
		}
	}

	// Set the value at the given memory address (CPU access, unavailable if reserved by the PPU)
	void OAMMapping::set(uint16_t address, uint8_t value) {
		if (accessible) {
			// Normal OAM access
			if (address < OFFSET_UNUSED) {
				m_array[address] = value;
			}

			// Unused 0xFEA0-0xFEFF area
			else {
				// CGB revision D
				if (m_hardware->isCGBConsole() && m_hardware->system() == SystemRevision::CGB_D) {
					if (address < OFFSET_FEC0)
						m_fea0[address - OFFSET_UNUSED] = value;
					else
						m_fec0[(address - OFFSET_FEC0) & 0x0F] = value;
				}

				// CGB revisions < D
				else if (m_hardware->isCGBConsole() && m_hardware->system() != SystemRevision::CGB_E) {
					if (address < OFFSET_FEC0)
						m_fea0[(address - OFFSET_UNUSED) & 7] = value;
					else if (address < OFFSET_FEE0)
						m_fec0[(address - OFFSET_FEC0) & 7] = value;
					else
						m_fee0[(address - OFFSET_FEE0) & 7] = value;
				}

				// On all other tested hardwares, writes are ignored
			}
		}
	}

	// Get the value at the given memory address (PPU access, unavailable if not reserved)
	uint8_t OAMMapping::lcdGet(uint16_t address) {
		if (accessible) return 0xFF;
		return m_array[address];
	}

	// Set the value at the given memory address (PPU access, unavailable if not reserved)
	void OAMMapping::lcdSet(uint16_t address, uint8_t value) {
		if (!accessible)
			m_array[address] = value;
	}
}
