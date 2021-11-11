#include "cart/mapping/MBC3CartMapping.hpp"

#define CARTTYPE_MBC3 0x11
#define CARTTYPE_MBC3_RAM 0x12
#define CARTTYPE_MBC3_RAM_BATTERY 0x13
#define CARTTYPE_MBC3_TIMER_BATTERY 0x0F
#define CARTTYPE_MBC3_RAM_TIMER_BATTERY 0x10

namespace toygb {
	MBC3CartMapping::MBC3CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {
		switch (carttype){
			case CARTTYPE_MBC3: setCartFeatures(false, false); break;
			case CARTTYPE_MBC3_RAM: setCartFeatures(true, false); break;
			case CARTTYPE_MBC3_RAM_BATTERY: setCartFeatures(true, true); break;
			case CARTTYPE_MBC3_TIMER_BATTERY: setCartFeatures(false, true); break;
			case CARTTYPE_MBC3_RAM_TIMER_BATTERY: setCartFeatures(true, true); break;
		}

		loadCartData();

		m_ramBankSelect = 0;
		m_romBankSelect = 1;
		if (m_ramData != nullptr){
			m_ramMapping = new BankedMemoryMapping(&m_ramBankSelect, SRAM_SIZE, m_ramData, false);
		} else {
			m_ramMapping = nullptr;
		}
	}

	MBC3CartMapping::~MBC3CartMapping(){

	}

	MemoryMapping* MBC3CartMapping::getRAM(){
		return m_ramMapping;
	}

	uint8_t MBC3CartMapping::get(uint16_t address){
		if (address < 0x4000){
			return m_romData[address];
		} else {
			return m_romData[m_romBankSelect * ROM_BANK_SIZE + (address - ROM0_SIZE)];
		}
	}

	void MBC3CartMapping::set(uint16_t address, uint8_t value){
		if (address < 0x2000 && m_ramMapping != nullptr){
			m_ramMapping->accessible = (value == 0x0A);
		} else if (0x2000 <= address && address < 0x4000){
			m_romBankSelect = value & 0x7F;
			if (m_romBankSelect == 0) m_romBankSelect = 1;
		} else if (0x4000 <= address && address < 0x6000){
			m_ramBankSelect = value & 0x0F;
		}
	}
}
