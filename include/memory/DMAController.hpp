#ifndef _MEMORY_DMACONTROLLER_HPP
#define _MEMORY_DMACONTROLLER_HPP

#include "core/hardware.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/OAMDMAMapping.hpp"
#include "util/error.hpp"
#include "util/component.hpp"


namespace toygb {
	class DMAController {
		public:
			DMAController();
			~DMAController();

			void configureMemory(MemoryMap* memory);
			void init(HardwareConfig& hardware);

			GBComponent run(MemoryMap* memory);

			bool isOAMDMAActive();

		private:
			HardwareConfig m_hardware;
			OAMDMAMapping* m_oamDmaMapping;
	};
}


#endif
