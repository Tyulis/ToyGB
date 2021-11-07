#include "Gameboy.hpp"
#include <iostream>

namespace toygb {
	Gameboy::Gameboy(GameboyConfig& config){
		m_config = config;
		m_interrupt = InterruptVector();
		m_cpu = CPU();
		m_lcd = LCDController();
		m_audio = AudioController();
		m_cart = CartController();
	}

	void Gameboy::init(OperationMode mode){
		m_cart.init(m_config.romfile, m_config.ramfile);
		if (mode == OperationMode::Auto) {
			mode = m_cart.getAutoOperationMode();
		}

		m_mode = mode;

		// Initialize components
		m_interrupt.init(mode);
		m_cpu.init(mode, &m_interrupt);
		m_lcd.init(mode, &m_interrupt);
		m_audio.init(mode);
		m_joypad.init(mode, &m_interrupt);

		// Build the memory map
		m_memory = MemoryMap();
		m_interrupt.configureMemory(&m_memory);
		m_cpu.configureMemory(&m_memory);
		m_lcd.configureMemory(&m_memory);
		m_audio.configureMemory(&m_memory);
		m_joypad.configureMemory(&m_memory);
		m_cart.configureMemory(&m_memory);
		m_memory.build();

		//std::cout << oh8(m_memory.get(0x0144)) << " " << oh8(m_memory.get(0xFF40)) << " " << oh8(m_memory.get(0x0180)) << std::endl;
	}

	void runCPU(CPU* cpu, MemoryMap* memory, clocktime_t startTime){
		(*cpu)(memory, startTime);
	}

	void runLCD(LCDController* lcd, clocktime_t startTime){
		(*lcd)(startTime);
	}

	void Gameboy::main() {
		clocktime_t startTime = std::chrono::steady_clock::now();
		// m_interface = Interface();
		std::thread cputhread(&runCPU, &m_cpu, &m_memory, startTime);
		std::thread lcdthread(&runLCD, &m_lcd, startTime);
		//std::thread audiothread(m_audio, m_startTime);

		lcdthread.join();
		cputhread.join();
		//audiothread.join();
		//m_cpu(&m_memory);
	}
}
