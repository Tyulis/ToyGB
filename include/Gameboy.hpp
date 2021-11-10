#ifndef _GAMEBOY_HPP
#define _GAMEBOY_HPP

#include <string>
#include <chrono>
#include <thread>

#include "GameboyConfig.hpp"
#include "core/CPU.hpp"
#include "core/timing.hpp"
#include "core/InterruptVector.hpp"
#include "core/OperationMode.hpp"
#include "cart/CartController.hpp"
#include "graphics/LCDController.hpp"
#include "audio/AudioController.hpp"
#include "control/JoypadController.hpp"
#include "ui/Interface.hpp"
#include "util/error.hpp"


namespace toygb {
	class Gameboy {
		public:
			Gameboy(GameboyConfig& config);

			void init(OperationMode mode);
			void main();

		private:
			GameboyConfig m_config;
			CPU m_cpu;
			LCDController m_lcd;
			InterruptVector m_interrupt;
			AudioController m_audio;
			JoypadController m_joypad;
			CartController m_cart;
			OperationMode m_mode;
			MemoryMap m_memory;
			Interface m_interface;
	};

	void runInterface(Interface* interface, LCDController* lcd, JoypadController* joypad);
}

#endif
