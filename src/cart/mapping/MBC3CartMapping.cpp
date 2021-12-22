#include "cart/mapping/MBC3CartMapping.hpp"

#define CARTTYPE_MBC3 0x11
#define CARTTYPE_MBC3_RAM 0x12
#define CARTTYPE_MBC3_RAM_BATTERY 0x13
#define CARTTYPE_MBC3_TIMER_BATTERY 0x0F
#define CARTTYPE_MBC3_RAM_TIMER_BATTERY 0x10

namespace toygb {
	// TODO : Implement real time clock
	// Initialize the memory mapping
	MBC3CartMapping::MBC3CartMapping(uint8_t carttype, std::string romfile, std::string ramfile) : ROMMapping(carttype, romfile, ramfile) {
		switch (carttype) {
			case CARTTYPE_MBC3: setCartFeatures(false, false); break;
			case CARTTYPE_MBC3_RAM: setCartFeatures(true, false); break;
			case CARTTYPE_MBC3_RAM_BATTERY: setCartFeatures(true, true); break;
			case CARTTYPE_MBC3_TIMER_BATTERY: setCartFeatures(false, true); break;
			case CARTTYPE_MBC3_RAM_TIMER_BATTERY: setCartFeatures(true, true); break;
		}

		loadCartData();

		// Default values at boot
		m_ramBankSelect = 0;
		m_romBankSelect = 1;

		if (m_ramData != nullptr)
			m_ramMapping = new FullBankedMemoryMapping(&m_ramBankSelect, m_ramSize/SRAM_SIZE, SRAM_SIZE, m_ramData, false);
		else
			m_ramMapping = nullptr;
	}

	MBC3CartMapping::~MBC3CartMapping() {

	}

	MemoryMapping* MBC3CartMapping::getRAM() {
		return m_ramMapping;
	}

	uint8_t MBC3CartMapping::get(uint16_t address) {
		if (address < 0x4000) {  // Fixed bank area
			return m_romData[address];
		} else {  // Switchable bank area
			return m_romData[m_romBankSelect * ROM_BANK_SIZE + (address - ROM0_SIZE)];
		}
	}

	void MBC3CartMapping::set(uint16_t address, uint8_t value){
		if (address < 0x2000 && m_ramMapping != nullptr) {  // 0x0000 - 0x1FFF : RAM enable, low 4 bits must be 0x0A to enable
			m_ramMapping->accessible = ((value & 0x0F) == 0x0A);
		} else if (0x2000 <= address && address < 0x4000) {  // 0x2000 - 0x3FFF : ROM bank select
			m_romBankSelect = value & 0x7F;
			if (m_romBankSelect == 0) m_romBankSelect = 1;  // Can’t select bank 0, select 1 instead
		} else if (0x4000 <= address && address < 0x6000) {  // 0x4000 - 0x5FFF : RAM bank select
			m_ramBankSelect = value & 0x0F;
		} else if (0x6000 <= address && address < 0x8000) {  // 0x6000 - 0x7FFF : Latch clock data
			// TODO : Implement RTC
		}
	}
}
