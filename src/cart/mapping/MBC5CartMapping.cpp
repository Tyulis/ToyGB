#include "cart/mapping/MBC5CartMapping.hpp"

#define CARTTYPE_MBC5 0x19
#define CARTTYPE_MBC5_RAM 0x1A
#define CARTTYPE_MBC5_RAM_BATTERY 0x1B
#define CARTTYPE_MBC5_RUMBLE 0x1C
#define CARTTYPE_MBC5_RUMBLE_RAM 0x1D
#define CARTTYPE_MBC5_RUMBLE_RAM_BATTERY 0x1E

namespace toygb {
	// TODO : Not implemented
	MBC5CartMapping::MBC5CartMapping(uint8_t carttype, std::string romfile, std::string ramfile, HardwareStatus* hardware) : ROMMapping(carttype, romfile, ramfile, hardware) {
		switch (carttype) {
			case CARTTYPE_MBC5: setCartFeatures(false, false, false); break;
			case CARTTYPE_MBC5_RAM: setCartFeatures(true, false, false); break;
			case CARTTYPE_MBC5_RAM_BATTERY: setCartFeatures(true, true, false); break;
			case CARTTYPE_MBC5_RUMBLE: setCartFeatures(false, false, false); break;
			case CARTTYPE_MBC5_RUMBLE_RAM: setCartFeatures(true, false, false); break;
			case CARTTYPE_MBC5_RUMBLE_RAM_BATTERY: setCartFeatures(true, true, false); break;
		}

		loadCartData();

		// Default values at boot
		m_ramBankSelect = 0;
		m_romBankSelect = 1;

		m_romBanks = m_romSize / ROM_BANK_SIZE;
		m_ramBanks = m_ramSize / SRAM_SIZE;

		if (m_ramData != nullptr) {
			m_ramMapping = new FullBankedMemoryMapping(&m_ramBankSelect, m_ramBanks, SRAM_SIZE, m_ramData, false);
			loadSaveData(m_ramMapping);
		} else {
			m_ramMapping = nullptr;
		}
	}

	MBC5CartMapping::~MBC5CartMapping() {

	}

	MemoryMapping* MBC5CartMapping::getRAM() {
		return m_ramMapping;
	}

	uint8_t MBC5CartMapping::get(uint16_t address) {
		// 0x0000-0x3FFF : Fixed bank area
		if (address < 0x4000)
			return m_romData[address];
		// 0x4000-0x7FFF : Switchable bank area
		else
			return m_romData[(address - 0x4000) + m_romBankSelect * ROM_BANK_SIZE];
	}

	void MBC5CartMapping::set(uint16_t address, uint8_t value) {
		// 0x0000-0x1FFF : RAM enable flag
		if (address < 0x2000) {
			m_ramMapping->accessible = ((value & 0x0F) == 0x0A);  // Lower 4 bits must be 0x0A to enable RAM
		}
		// 0x2000-0x2FFF : Lower byte of switchable ROM bank index
		else if (0x2000 <= address && address < 0x3000) {
			m_romBankSelect = (m_romBankSelect & 0x100) | value;
		}
		// 0x3000-0x3FFF : Upper bit of switchable ROM bank index
		else if (0x3000 <= address && address < 0x4000) {
			m_romBankSelect = (m_romBankSelect & 0x0FF) | ((value & 1) << 8);
		}
		// 0x4000-0x5FFF : RAM bank select
		else if (0x4000 <= address && address < 0x6000) {
			m_ramBankSelect = value & 0x0F;  // FIXME : check the index or just mask the excess bits out ?
		}
	}
}
