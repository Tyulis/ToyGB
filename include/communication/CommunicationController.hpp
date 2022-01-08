#ifndef _COMMUNICATION_COMMUNICATIONCONTROLLER_HPP
#define _COMMUNICATION_COMMUNICATIONCONTROLLER_HPP

#include "communication/mapping/InfraredTransferMapping.hpp"
#include "communication/mapping/SerialTransferMapping.hpp"
#include "core/hardware.hpp"
#include "core/InterruptVector.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Serial communications components
	 * TODO : Implement actual communications ? Currently this is just a stub to have a working interface to the CPU */
	class CommunicationController {
		public:
			CommunicationController();
			~CommunicationController();

			void init(HardwareStatus* hardware, InterruptVector* interrupt);
			void configureMemory(MemoryMap* memory);

		private:
			HardwareStatus* m_hardware;
			InterruptVector* m_interrupt;
			SerialTransferMapping* m_serialMapping;
			InfraredTransferMapping* m_infraredMapping;

	};
}

#endif
