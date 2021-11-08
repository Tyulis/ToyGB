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

	void Gameboy::main() {
		GBComponent cpuComponent = m_cpu.run(&m_memory);
		GBComponent lcdComponent = m_lcd.run();

		clocktime_t startTime = std::chrono::steady_clock::now();
		int64_t lastCycle = 0;
		while (true){
			cpuComponent.onCycle();
			lcdComponent.onCycle();

			int64_t target = lastCycle + CLOCK_CYCLE_NS;
			while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - startTime).count() < target);
			lastCycle = target;
		}
	}
}
