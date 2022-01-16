#ifndef _MEMORY_DMACONTROLLER_HPP
#define _MEMORY_DMACONTROLLER_HPP

#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/OAMDMAMapping.hpp"
#include "util/error.hpp"
#include "util/component.hpp"


namespace toygb {
	/** Implementation of the DMA component */
	class DMAController {
		public:
			DMAController();
			~DMAController();

			void configureMemory(MemoryMap* memory);
			void init(HardwareStatus* hardware);

			/** Main component, called every 4 clocks */
			void runCycle();

			bool isOAMDMAActive() const;                  // Tell whether an OAM DMA operation is active
			bool isConflicting(uint16_t address) const;   // Tell whether a bus conflict with OAM DMA can occur at the given address
			uint8_t conflictingRead(uint16_t address);    // Get the value that the CPU will read at the given address, accounting for bus conflicts

		private:
			HardwareStatus* m_hardware;
			OAMDMAMapping* m_oamDmaMapping;
			MemoryMap* m_memory;

			int m_cyclesToSkip;
	};
}


#endif
