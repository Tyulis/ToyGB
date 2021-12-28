#ifndef _MEMORY_MAPPING_OAMDMAMAPPING_HPP
#define _MEMORY_MAPPING_OAMDMAMAPPING_HPP

#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** OAM DAM IO register memory mapping */
	class OAMDMAMapping : public MemoryMapping {
		public:
			OAMDMAMapping(HardwareConfig& hardware);

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			uint16_t sourceAddress;  // Source address (register DMA << 8), then contains the next address to transfer
			bool active;             // Whether an OAM DMA operation is active

		private:
			HardwareConfig m_hardware;
	};
}

#endif
