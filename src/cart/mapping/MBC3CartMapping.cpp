#include "cart/mapping/MBC3CartMapping.hpp"

#define CARTTYPE_MBC3 0x11
#define CARTTYPE_MBC3_RAM 0x12
#define CARTTYPE_MBC3_RAM_BATTERY 0x13
#define CARTTYPE_MBC3_TIMER_BATTERY 0x0F
#define CARTTYPE_MBC3_RAM_TIMER_BATTERY 0x10

namespace toygb {
	// TODO : Implement real time clock
	// Initialize the memory mapping
	MBC3CartMapping::MBC3CartMapping(uint8_t carttype, std::string romfile, std::string ramfile, HardwareStatus* hardware) : ROMMapping(carttype, romfile, ramfile, hardware) {
		switch (carttype) {
			case CARTTYPE_MBC3: setCartFeatures(false, false, false); break;
			case CARTTYPE_MBC3_RAM: setCartFeatures(true, false, false); break;
			case CARTTYPE_MBC3_RAM_BATTERY: setCartFeatures(true, true, false); break;
			case CARTTYPE_MBC3_TIMER_BATTERY: setCartFeatures(false, true, true); break;
			case CARTTYPE_MBC3_RAM_TIMER_BATTERY: setCartFeatures(true, true, true); break;
		}

		loadCartData();

		if (m_hasRTC) {
			// FIXME : does the RTC read garbage when unititialized ?
			m_rtc = new MBC3RTC {.divider = 0, .seconds = 0, .minutes = 0, .hours = 0, .days = 0, .dayCarry = 0, .halt = false};
			m_rtcLatch = new MBC3RTC {.divider = 0, .seconds = 0, .minutes = 0, .hours = 0, .days = 0, .dayCarry = 0, .halt = false};
		} else {
			m_rtc = nullptr;
			m_rtcLatch = nullptr;
		}
		m_rtcLatched = false;

		// Default values at boot
		m_ramBankSelect = 0;
		m_romBankSelect = 1;

		if (m_ramData != nullptr) {
			m_ramMapping = new MBC3RAMMapping(&m_ramBankSelect, m_ramSize/SRAM_SIZE, SRAM_SIZE, m_ramData, false, m_rtc, m_rtcLatch);
			loadSaveData(m_ramMapping);
		} else {
			m_ramMapping = nullptr;
		}
	}

	MBC3CartMapping::~MBC3CartMapping() {
		if (m_rtc != nullptr) delete m_rtc;
		if (m_rtcLatch != nullptr) delete m_rtcLatch;
		m_rtc = m_rtcLatch = nullptr;
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
		} else if (0x6000 <= address && address < 0x8000 && m_hasRTC) {  // 0x6000 - 0x7FFF : Latch clock data
			// Need to write 0 then 1 to latch the registers
			if (!m_rtcLatched && (value & 1)) {
				m_rtcLatch->divider = m_rtc->divider;
				m_rtcLatch->seconds = m_rtc->seconds;
				m_rtcLatch->minutes = m_rtc->minutes;
				m_rtcLatch->hours = m_rtc->hours;
				m_rtcLatch->days = m_rtc->days;
				m_rtcLatch->halt = m_rtc->halt;
				m_rtcLatch->dayCarry = m_rtc->dayCarry;
			}
			m_rtcLatched = value & 1;
		}
	}

	// Update the RTC, this is called at every clock cycle
	// Here, while the emulator is running, the RTC is tied to the global Gameboy clock
	// This is technically inaccurate as it is actually an independant 32768Hz oscillator located in the cartridge,
	// but in that case using an external RTC (like the system clock) would be sensitive to software lag during emulation,
	// leading to unwanted desynchronizations between the Gameboy and the RTC
	void MBC3CartMapping::update() {
		if (m_hasRTC) {
			if (m_rtc->halt)
				return;

			// Use the emulator sequencer (that continuously ticks at the CPU clock rate), not the Gameboy divider (sensitive to the program’s behaviour)
			uint16_t sequencer = m_hardware->getSequencer();

			// In single-speed mode, sequencer ticks at 4194304Hz and we want a 32768Hz clock, so one RTC tick every 128 sequencer ticks
			// In double-speed mode, sequencer ticks twice as fast so one RTC tick every 256 sequencer ticks
			if ((sequencer & (m_hardware->doubleSpeed() ? 0xFF : 0x7F)) == 0) {
				// Overflow into the seconds register every 32768 ticks at 32768Hz
				m_rtc->divider = (m_rtc->divider + 1) & 0x7FFF;
				if (m_rtc->divider == 0) {
					m_rtc->seconds = (m_rtc->seconds + 1) % 60;
					// Just update each register in cascade as the immediately lower register overflows
					if (m_rtc->seconds == 0) {
						m_rtc->minutes = (m_rtc->minutes + 1) % 60;
						if (m_rtc->minutes == 0) {
							m_rtc->hours = (m_rtc->hours + 1) % 24;
							if (m_rtc->hours == 0) {
								m_rtc->days = (m_rtc->days + 1) % 512;
								// When the days overflow, set the day carry
								if (m_rtc->days == 0) {
									m_rtc->dayCarry = 1;
								}
							}
						}
					}
				}
			}
		}
	}
}
