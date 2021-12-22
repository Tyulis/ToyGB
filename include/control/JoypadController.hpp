#ifndef _CONTROL_JOYPADCONTROLLER_HPP
#define _CONTROL_JOYPADCONTROLLER_HPP

#include "control/mapping/JoypadMapping.hpp"
#include "core/hardware.hpp"
#include "core/InterruptVector.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"


namespace toygb {
	/** Joypad components
	 * This makes the interface between the emulators’s UI signals and the GB register */
	class JoypadController {
		public:
			JoypadController();
			~JoypadController();

			void init(HardwareConfig& hardware, InterruptVector* interrupt);
			void configureMemory(MemoryMap* memory);

			/** Set the given button status from the interface (JoypadButton enum is defined in control/mapping/JoypadMapping.hpp) */
			void setButton(JoypadButton button, bool pressed);

		private:
			HardwareConfig m_hardware;
			InterruptVector* m_interrupt;

			JoypadMapping* m_register;
	};
}

#endif
