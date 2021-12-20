#ifndef _COMMUNICATION_SERIALCONTROLLER_HPP
#define _COMMUNICATION_SERIALCONTROLLER_HPP

#include "util/error.hpp"
#include "core/hardware.hpp"
#include "core/InterruptVector.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/SerialTransferMapping.hpp"


namespace toygb {
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
