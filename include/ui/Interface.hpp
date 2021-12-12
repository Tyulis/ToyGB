#ifndef _UI_INTERFACE_HPP
#define _UI_INTERFACE_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "audio/AudioController.hpp"
#include "graphics/LCDController.hpp"
#include "control/JoypadController.hpp"
#include "ui/GBAudioStream.hpp"

namespace toygb {
	class Interface {
		public:
			Interface();

			void run(LCDController* lcd, AudioController* audio, JoypadController* joypad);

		private:
			void setupAudio();
			void updateJoypad();
			void updateGraphics(sf::Uint8* pixels);

			LCDController* m_lcd;
			AudioController* m_audio;
			JoypadController* m_joypad;
			GBAudioStream m_audioStream;
	};
}

#endif
