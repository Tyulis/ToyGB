#ifndef _UI_INTERFACE_HPP
#define _UI_INTERFACE_HPP

#include <SFML/Graphics.hpp>

#include "graphics/LCDController.hpp"
#include "control/JoypadController.hpp"

namespace toygb {
	class Interface {
		public:
			Interface();

			void run(LCDController* lcd, JoypadController* joypad);

		private:
			LCDController* m_lcd;
			JoypadController* m_joypad;
	};
}

#endif
