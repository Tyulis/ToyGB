#include "cart/mapping/MBC1CartMapping.hpp"

#define CARTTYPE_MBC1 0x01
#define CARTTYPE_MBC1_RAM 0x02
#define CARTTYPE_MBC1_RAM_BATTERY 0x03

namespace toygb {
	// Initialize the MBC1 ROM mapping
	MBC1CartMapping::MBC1CartMapping(uint8_t carttype, std::string romfile, std::string ramfile, HardwareStatus* hardware) : ROMMapping(carttype, romfile, ramfile, hardware) {
		switch (carttype) {
			case CARTTYPE_MBC1: setCartFeatures(false, false, false); break;
			case CARTTYPE_MBC1_RAM: setCartFeatures(true, false, false); break;
			case CARTTYPE_MBC1_RAM_BATTERY: setCartFeatures(true, true, false); break;
		}

		loadCartData();

		// Default values at boot
		m_ramBankSelect = 0;
		m_romBankSelect = 1;
		m_modeSelect = false;

		// Has RAM (does not check the hasRAM flag because some homebrew roms do not set it in the cartridge header, a real gameboy doesn’t care)
		if (m_ramData != nullptr) {
			m_ramMapping = new MBC1RAMMapping(&m_ramBankSelect, &m_modeSelect, m_ramSize/SRAM_SIZE, SRAM_SIZE, m_ramData, false);
		} else {
			m_ramMapping = nullptr;
		}

		m_romBanks = m_romSize / ROM_BANK_SIZE;
		m_ramBanks = m_ramSize / SRAM_SIZE;

		// Compute the mask to apply onto the selected rom bank
		// If a rom bank greater than the amount of banks present in the cart is selected, the additional bits are masked out
		// (This just builds the mask from the number of banks, for example 0b00101010 -> 0b00111111)
		m_romBankMask = (1 << int(std::floor(std::log2(m_romBanks - 1) + 1))) - 1;
	}

	MBC1CartMapping::~MBC1CartMapping() {

	}

	// Return the associated cartridge RAM mapping
	MemoryMapping* MBC1CartMapping::getRAM() {
		return m_ramMapping;
	}

	// Get the value at the given relative address (0 = first address of the mapping)
	uint8_t MBC1CartMapping::get(uint16_t address) {
		// First ROM area
		if (address < 0x4000) {
			// Mode 1 : advanced banking mode : even base section is affected by the additional banking register,
			//          so it may switch between 0x00, 0x20, 0x40, 0x60 depending on m_ramBankSelect
			if (m_modeSelect) {
				// Final bank = (m_ramBankSelect * 0x20 + 0x00), with excess bits masked out
				return m_romData[address + ((m_ramBankSelect << 5) & m_romBankMask) * ROM_BANK_SIZE];
			} else {  // Mode 0 : simple ROM banking : just get the value in bank 0
				return m_romData[address];
			}
		// Switchable ROM area
		} else {
			// Final bank = (m_ramBankSelect * 0x20 + m_romBankSelect), with excess bits masked out
			// case m_romBankSelect == 0 is handled in MBC1CartMapping::set
			int bank = ((m_ramBankSelect << 5) | m_romBankSelect) & m_romBankMask;
			return m_romData[(address - 0x4000) + bank * ROM_BANK_SIZE];
		}
	}

	// Set the value at the given relative address
	void MBC1CartMapping::set(uint16_t address, uint8_t value){
		if (address < 0x2000 && m_ramMapping != nullptr){  // 0x0000 - 0x1FFF : RAM enable
			m_ramMapping->accessible = ((value & 0x0F) == 0x0A);  // Lower 4 bits must be 0x0A
		} else if (0x2000 <= address && address < 0x4000){  // 0x2000 - 0x3FFF : switchable ROM bank select
			m_romBankSelect = value;
			if (m_romBankSelect == 0) m_romBankSelect = 1;  // Can’t map bank 0 to the switchable area, map 1 instead
		} else if (0x4000 <= address && address < 0x6000){  // 0x4000 - 0x5FFF : RAM bank select / upper ROM bank bits
			m_ramBankSelect = value & 0x03;
		} else if (0x6000 <= address && address < 0x8000){  // 0x6000 - 0x7FFF : Mode select
			m_modeSelect = value & 1;
		}
	}
}
