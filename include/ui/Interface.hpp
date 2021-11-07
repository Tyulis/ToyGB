#ifndef _UI_INTERFACE_HPP
#define _UI_INTERFACE_HPP

#include "graphics/LCDController.hpp"
#include "control/JoypadController.hpp"

namespace toygb {
	class Interface {
		public:
			Interface();

			void operator()(LCDController* lcd, JoypadController* joypad);

		private:
			LCDController* m_lcd;
			JoypadController* m_joypad;
	};
}

#endif
