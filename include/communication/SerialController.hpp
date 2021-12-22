#ifndef _COMMUNICATION_SERIALCONTROLLER_HPP
#define _COMMUNICATION_SERIALCONTROLLER_HPP

#include "communication/mapping/SerialTransferMapping.hpp"
#include "core/hardware.hpp"
#include "core/InterruptVector.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Serial communications components
	 * TODO : Implement actual communications ? Currently this is just a stub to have a working interface to the CPU */
	class SerialController {
		public:
			SerialController();
			~SerialController();

			void init(HardwareConfig& hardware, InterruptVector* interrupt);
			void configureMemory(MemoryMap* memory);

		private:
			HardwareConfig m_hardware;
			InterruptVector* m_interrupt;
			SerialTransferMapping* m_mapping;

	};
}

#endif
