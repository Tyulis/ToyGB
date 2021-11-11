#ifndef _UI_INTERFACE_HPP
#define _UI_INTERFACE_HPP

#include <SFML/Graphics.hpp>

#include "graphics/LCDController.hpp"
#include "control/JoypadController.hpp"

namespace toygb {
	class Interface {
		public:
			Interface();

			void run(LCDController* lcd, AudioController* audio, JoypadController* joypad);

		private:
			LCDController* m_lcd;
			AudioController* m_audio;
			JoypadController* m_joypad;
	};
}

#endif
