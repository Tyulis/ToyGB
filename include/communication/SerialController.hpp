#ifndef _COMMUNICATION_SERIALCONTROLLER_HPP
#define _COMMUNICATION_SERIALCONTROLLER_HPP

#include "util/error.hpp"
#include "core/OperationMode.hpp"
#include "core/InterruptVector.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/SerialTransferMapping.hpp"


namespace toygb {
	class SerialController {
		public:
			SerialController();
			~SerialController();

			void init(OperationMode mode, InterruptVector* interrupt);
			void configureMemory(MemoryMap* memory);

		private:
			OperationMode m_mode;
			InterruptVector* m_interrupt;
			SerialTransferMapping* m_mapping;

	};
}

#endif
