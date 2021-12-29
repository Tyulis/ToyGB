#ifndef _MEMORY_DMACONTROLLER_HPP
#define _MEMORY_DMACONTROLLER_HPP

#include "core/hardware.hpp"
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
			void init(HardwareConfig* hardware);

			/** Main loop of the component, as a coroutine */
			GBComponent run(MemoryMap* memory);

			/** Tell whether the emulator can skip the component on that cycle, to save a context commutation */
			bool skip() const;

			bool isOAMDMAActive();  // Tell whether an OAM DMA operation is active

		private:
			HardwareConfig* m_hardware;
			OAMDMAMapping* m_oamDmaMapping;
	};
}


#endif
