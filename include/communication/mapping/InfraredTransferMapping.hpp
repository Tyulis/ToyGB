#ifndef _COMMUNICATION_MAPPING_INFRAREDTRANSFERMAPPING_HPP
#define _COMMUNICATION_MAPPING_INFRAREDTRANSFERMAPPING_HPP

#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Infrared communication IO register mapping */
	class InfraredTransferMapping : public MemoryMapping {
		public:
			InfraredTransferMapping(HardwareStatus* m_hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			bool dataWrite;          // Bit to transmit (0 = LED off, 1 = LED on) (register RP, bit 0)
			bool dataRead;           // Set whether a transfer is requested / in progress (register RP, bit 1)
			uint8_t dataReadEnable;  // Set whether to use the fast clock speed (3 = read, 0 = don’t) (register RP, bit 6-7)

		private:
			HardwareStatus* m_hardware;
	};
}

#endif
