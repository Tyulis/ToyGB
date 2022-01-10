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
			OAMDMAMapping(HardwareStatus* hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			uint16_t sourceAddress;  // Source address (register DMA << 8), then contains the next address to transfer
			bool active;             // Whether an OAM DMA operation is actually active (and actively copying)

			int idleCycles;             // Number of remaining idle clocks before actually starting DMA
			bool requested;             // Whether a new DMA was requested by writing to FF46, and waiting 4 clocks before actually (re-)starting DMA
			uint16_t requestedAddress;  // Address that was requested at the last write to FF46 (to keep it during the startup cycles, as if a previous DMA was running it continues in those 4 clocks)

		private:
			HardwareStatus* m_hardware;
	};
}

#endif
