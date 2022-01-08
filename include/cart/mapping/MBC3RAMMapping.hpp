#ifndef _CART_MAPPING_MBC3RAMMAPPING_HPP
#define _CART_MAPPING_MBC3RAMMAPPING_HPP

#include <iostream>

#include "core/timing.hpp"
#include "memory/mapping/FullBankedMemoryMapping.hpp"


namespace toygb {
	typedef struct {
		uint16_t divider;  // The RTC is clocked by a 32768Hz oscillator
		uint8_t seconds;
		uint8_t minutes;
		uint8_t hours;
		uint16_t days;
		bool dayCarry;
		bool halt;
	} MBC3RTC;

	/** MBC3 SRAM and RTC registers memory mapping */
	class MBC3RAMMapping : public FullBankedMemoryMapping {
		public:
			/** Initialize the memory mapping
			 * uint8_t* bankSelect : Pointer to the RAM bank select register
			 * int numBanks        : Total amount of RAM banks
			 * uint16_t bankSize   : Size of one RAM bank, in bytes
			 * uint8_t* array      : RAM content, must be of size (numBanks * bankSize)
			 * bool accessible     : Whether the memory mapping is accessible initially
			 * MBC3RTC rtc         : Value of the actual RTC registers (nullptr if there is no RTC)
			 * MBC3RTC rtcLatch    : Latched RTC registers values (nullptr if there is no RTC) */
			MBC3RAMMapping(uint8_t* bankSelect, int numBanks, uint16_t bankSize, uint8_t* array, bool accessible, MBC3RTC* rtc, MBC3RTC* rtcLatch);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			virtual void load(std::istream& input);
			virtual void save(std::ostream& output);

		protected:
			MBC3RTC* m_rtc;
			MBC3RTC* m_rtcLatch;
	};
}

#endif
