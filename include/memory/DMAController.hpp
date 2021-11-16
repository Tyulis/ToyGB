#ifndef _MEMORY_DMACONTROLLER_HPP
#define _MEMORY_DMACONTROLLER_HPP

#include "util/error.hpp"
#include "util/component.hpp"
#include "core/OperationMode.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/OAMDMAMapping.hpp"


namespace toygb {
	class DMAController {
		public:
			DMAController();
			~DMAController();

			void configureMemory(MemoryMap* memory);
			void init(OperationMode mode);

			GBComponent run(MemoryMap* memory);

			bool isOAMDMAActive();

		private:
			OperationMode m_mode;
			OAMDMAMapping* m_oamDmaMapping;
	};
}


#endif
