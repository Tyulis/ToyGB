#ifndef _CONTROL_JOYPADCONTROLLER_HPP
#define _CONTROL_JOYPADCONTROLLER_HPP

#include "core/hardware.hpp"
#include "core/InterruptVector.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/JoypadMapping.hpp"


namespace toygb {
	class JoypadController {
		public:
			JoypadController();
			~JoypadController();

			void init(HardwareConfig& hardware, InterruptVector* interrupt);
			void configureMemory(MemoryMap* memory);

			void setButton(JoypadButton button, bool pressed);

		private:
			HardwareConfig m_hardware;
			InterruptVector* m_interrupt;

			JoypadMapping* m_register;
	};
}

#endif
