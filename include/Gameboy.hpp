#ifndef _GAMEBOY_HPP
#define _GAMEBOY_HPP

#include <string>
#include <chrono>
#include <thread>

#include "GameboyConfig.hpp"
#include "audio/AudioController.hpp"
#include "cart/CartController.hpp"
#include "communication/CommunicationController.hpp"
#include "control/JoypadController.hpp"
#include "core/CPU.hpp"
#include "core/timing.hpp"
#include "core/hardware.hpp"
#include "core/InterruptVector.hpp"
#include "graphics/LCDController.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/DMAController.hpp"
#include "ui/Interface.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Main emulator class, manages the main memory map, clock and components */
	class Gameboy {
		public:
			Gameboy(GameboyConfig& config);
			void main();  // Main emulator loop

		private:
			GameboyConfig m_config;
			CPU m_cpu;
			LCDController m_lcd;
			InterruptVector m_interrupt;
			AudioController m_audio;
			JoypadController m_joypad;
			CartController m_cart;
			CommunicationController m_serial;
			DMAController m_dma;

			HardwareStatus m_hardware;
			MemoryMap m_memory;
			Interface m_interface;
	};

	void runInterface(Interface* interface, LCDController* lcd, AudioController* audio, JoypadController* joypad);
}

#endif
