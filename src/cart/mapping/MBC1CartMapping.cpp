#include "cart/mapping/MBC1CartMapping.hpp"

#define CARTTYPE_MBC1 0x01
#define CARTTYPE_MBC1_RAM 0x02
#define CARTTYPE_MBC1_RAM_BATTERY 0x03

namespace toygb {
	MBC1CartMapping::MBC1CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile){
		switch (carttype){
			case CARTTYPE_MBC1: setCartFeatures(false, false); break;
			case CARTTYPE_MBC1_RAM: setCartFeatures(true, false); break;
			case CARTTYPE_MBC1_RAM_BATTERY: setCartFeatures(true, true); break;
		}

		loadCartData();

		m_ramBankSelect = 0;
		m_romBankSelect = 1;
		m_modeSelect = false;
		if (m_ramData != nullptr){
			m_ramMapping = new MBC1RAMMapping(&m_ramBankSelect, &m_modeSelect, SRAM_SIZE, m_ramData, false);
		} else {
			m_ramMapping = nullptr;
		}
		m_romBanks = m_romSize / ROM_BANK_SIZE;
		m_ramBanks = m_ramSize / SRAM_SIZE;

		m_romBankMask = 0;
		uint8_t masktmp = m_romBanks;
		while (masktmp > 0){
			m_romBankMask = (masktmp << 1) | 1;
			masktmp >>= 1;
		}
	}

	MBC1CartMapping::~MBC1CartMapping(){

	}

	MemoryMapping* MBC1CartMapping::getRAM(){
		return m_ramMapping;
	}

	uint8_t MBC1CartMapping::get(uint16_t address){
		if (address < 0x4000){
			if (m_modeSelect){
				return m_romData[address + ((m_ramBankSelect << 5) & m_romBankMask) * ROM_BANK_SIZE];
			} else {
				return m_romData[address];
			}
		} else {
			int bank = ((m_ramBankSelect << 5) | m_romBankSelect) & m_romBankMask;
			return m_romData[address + (bank - 1) * ROM_BANK_SIZE];
		}
	}

	void MBC1CartMapping::set(uint16_t address, uint8_t value){
		if (address < 0x2000 && m_ramMapping != nullptr){
			m_ramMapping->accessible = ((value & 0x0F) == 0x0A);
		} else if (0x2000 <= address && address < 0x4000){
			m_romBankSelect = value;
			if (m_romBankSelect == 0) m_romBankSelect = 1;
		} else if (0x4000 <= address && address < 0x6000){
			m_ramBankSelect = value & 0x03;
		} else if (0x6000 <= address && address < 0x8000){
			m_modeSelect = value & 1;
		}
	}
}
