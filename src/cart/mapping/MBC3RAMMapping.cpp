#include "cart/mapping/MBC3RAMMapping.hpp"

#define RTC_BANK_SECONDS 0x08
#define RTC_BANK_MINUTES 0x09
#define RTC_BANK_HOURS   0x0A
#define RTC_BANK_DAYS    0x0B
#define RTC_BANK_CONTROL 0x0C


namespace toygb {
	// Initialize the memory mapping
	MBC3RAMMapping::MBC3RAMMapping(uint8_t* bankSelect, int numBanks, uint16_t bankSize, uint8_t* array, bool allowAccess, MBC3RTC* rtc, MBC3RTC* rtcLatch) :
					FullBankedMemoryMapping(bankSelect, numBanks, bankSize, array, allowAccess) {
		m_rtc = rtc;
		m_rtcLatch = rtcLatch;
	}

	// Get the value at the given relative address
	uint8_t MBC3RAMMapping::get(uint16_t address) {
		if (accessible) {
			// Cartridge RAM banks
			if (*m_bankSelect < m_numBanks) {
				return m_array[address + (*m_bankSelect) * m_bankSize];
			}

			// RTC registers
			else if (m_rtc != nullptr) {
				switch (*m_bankSelect) {
					case RTC_BANK_SECONDS:
						return m_rtcLatch->seconds;
					case RTC_BANK_MINUTES:
						return m_rtcLatch->minutes;
					case RTC_BANK_HOURS:
						return m_rtcLatch->hours;
					case RTC_BANK_DAYS:
						return m_rtcLatch->days & 0xFF;
					case RTC_BANK_CONTROL:
						return ((m_rtcLatch->days >> 8) & 1) | (m_rtcLatch->halt << 6) | (m_rtcLatch->dayCarry << 7);
					default:
						std::stringstream errstream;
						errstream << "Read from unknown MBC3 RAM bank (tried to read from bank " << oh8(*m_bankSelect) << ", banks 0x00 to 0x" << oh8(m_numBanks) << " + 0x08 to 0x0C available)";
						throw EmulationError(errstream.str());
				}
			}
		}

		return 0xFF;
	}

	void MBC3RAMMapping::set(uint16_t address, uint8_t value) {
		if (accessible) {
			// Cartridge RAM banks
			if (*m_bankSelect < m_numBanks) {
				m_array[address + (*m_bankSelect) * m_bankSize] = value;
			}

			// RTC registers
			else if (m_rtc != nullptr) {
				// FIXME : Does writing to the registers also update the latched values ?
				// FIXME : What happens when writing to the registers without halting the clock ?
				// FIXME : What happens when writing out-of-bounds values to a register (like a value >59 in the seconds)
				switch (*m_bankSelect) {
					case RTC_BANK_SECONDS:
						m_rtc->seconds = value;
						break;
					case RTC_BANK_MINUTES:
						m_rtc->minutes = value;
						break;
					case RTC_BANK_HOURS:
						m_rtc->hours = value;
						break;
					case RTC_BANK_DAYS:
						m_rtc->days = (m_rtc->days & 0x100) | value;
						break;
					case RTC_BANK_CONTROL:
						m_rtc->days = (m_rtc->days & 0xFF) | ((value & 1) << 8);
						m_rtc->halt = (value >> 6) & 1;
						m_rtc->dayCarry = (value >> 7) & 1;
						break;
					default:
						std::stringstream errstream;
						errstream << "Write to unknown MBC3 RAM bank (tried to write to bank " << oh8(*m_bankSelect) << ", banks 0x00 to 0x" << oh8(m_numBanks) << " + 0x08 to 0x0C available)";
						throw EmulationError(errstream.str());
				}
			}
		}
	}

	// Load the memory mapping's state from a file (for cartridge RAM save or save states)
	void MBC3RAMMapping::load(std::istream& input) {
		input.read(reinterpret_cast<char*>(m_array), m_numBanks*m_bankSize);

		// Read BESS-like RTC data at the end of the save file
		if (m_rtc != nullptr) {
			uint8_t seconds, minutes, hours, dayLow, control;
			uint64_t saveTimestamp;
			input.read(reinterpret_cast<char*>(&seconds), 1);
			input.seekg(3, std::istream::cur);
			input.read(reinterpret_cast<char*>(&minutes), 1);
			input.seekg(3, std::istream::cur);
			input.read(reinterpret_cast<char*>(&hours), 1);
			input.seekg(3, std::istream::cur);
			input.read(reinterpret_cast<char*>(&dayLow), 1);
			input.seekg(3, std::istream::cur);
			input.read(reinterpret_cast<char*>(&control), 1);
			input.seekg(3, std::istream::cur);
			input.read(reinterpret_cast<char*>(&saveTimestamp), 8);

			int64_t currentSeconds, saveSeconds = 86400*(((control & 1) << 8) & dayLow) + 3600*hours + 60*minutes + seconds;
			if ((control >> 6) & 1) {  // RTC halted, did not tick since last save
				currentSeconds = saveSeconds;
			} else {
				std::chrono::steady_clock::duration unixTime = std::chrono::seconds(saveTimestamp);
				clocktime_t saveTime(unixTime);
				clocktime_t currentTime = std::chrono::steady_clock::now();
				currentSeconds = saveSeconds + std::chrono::duration_cast<std::chrono::seconds>(currentTime - saveTime).count();
			}

			m_rtc->divider = 0;  // TODO : save the 32768Hz divider status ?
			m_rtc->seconds = currentSeconds % 60;
			m_rtc->minutes = (currentSeconds / 60) % 60;
			m_rtc->hours = (currentSeconds / 3600) % 24;
			m_rtc->days = (currentSeconds / 86400) % 512;
			m_rtc->halt = (control >> 6) & 1;
			m_rtc->dayCarry = (control >> 7) & 1;
		}
	}

	// Store the memory mapping's state in a file (for cartridge RAM save or save states)
	void MBC3RAMMapping::save(std::ostream& output) {
		output.write(reinterpret_cast<char*>(m_array), m_numBanks*m_bankSize);

		// Write BESS-like RTC data at the end of the save file
		if (m_rtc != nullptr) {
			uint8_t threePaddingBytes[3] = {0, 0, 0};
			int64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
			uint8_t dayLow = m_rtc->days & 0xFF;
			uint8_t control = ((m_rtc->days >> 8) & 1) | (m_rtc->halt << 6) | (m_rtc->dayCarry << 7);
			output.write(reinterpret_cast<char*>(&(m_rtc->seconds)), 1);
			output.write(reinterpret_cast<char*>(threePaddingBytes), 3);
			output.write(reinterpret_cast<char*>(&(m_rtc->minutes)), 1);
			output.write(reinterpret_cast<char*>(threePaddingBytes), 3);
			output.write(reinterpret_cast<char*>(&(m_rtc->hours)), 1);
			output.write(reinterpret_cast<char*>(threePaddingBytes), 3);
			output.write(reinterpret_cast<char*>(&(dayLow)), 1);
			output.write(reinterpret_cast<char*>(threePaddingBytes), 3);
			output.write(reinterpret_cast<char*>(&(control)), 1);
			output.write(reinterpret_cast<char*>(threePaddingBytes), 3);
			output.write(reinterpret_cast<char*>(&(timestamp)), 8);
		}
	}
}
