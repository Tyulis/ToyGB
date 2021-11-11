#ifndef _CONTROL_JOYPADCONTROLLER_HPP
#define _CONTROL_JOYPADCONTROLLER_HPP

#include "core/InterruptVector.hpp"
#include "core/OperationMode.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/JoypadMapping.hpp"


namespace toygb {
	class JoypadController {
		public:
			JoypadController();
			~JoypadController();

			void init(OperationMode mode, InterruptVector* interrupt);
			void configureMemory(MemoryMap* memory);

			void setButton(JoypadButton button, bool pressed);

		private:
			OperationMode m_mode;
			InterruptVector* m_interrupt;

			JoypadMapping* m_register;
	};
}

#endif
