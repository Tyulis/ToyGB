#ifndef _COMMUNICATION_MAPPING_SERIALTRANSFERMAPPING_HPP
#define _COMMUNICATION_MAPPING_SERIALTRANSFERMAPPING_HPP

#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Serial port communication IO registers mapping */
	class SerialTransferMapping : public MemoryMapping {
		public:
			SerialTransferMapping(HardwareStatus* m_hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			uint8_t transferData;    // Data to be transferred / that has been received through the serial port (register SB)
			bool transferStartFlag;  // Set whether a transfer is requested / in progress (register SC, bit 7)
			bool clockSpeed;         // Set whether to use the fast clock speed (CGB only) (register SC, bit 1)
			bool shiftClock;         // Clock to use (0 = external, 1 = internal) (register SC, bit 0)

		private:
			HardwareStatus* m_hardware;
	};
}

#endif
