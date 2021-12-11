#include "Gameboy.hpp"
#include <iostream>

namespace toygb {
	Gameboy::Gameboy(GameboyConfig& config):
		m_config(config), m_cpu(config.disassemble), m_lcd(),
		m_interrupt(), m_audio(), m_cart(), m_serial(), m_dma(){
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
		m_serial.init(mode, &m_interrupt);
		m_dma.init(mode);

		// Build the memory map
		m_memory = MemoryMap();
		m_interrupt.configureMemory(&m_memory);
		m_cpu.configureMemory(&m_memory);
		m_lcd.configureMemory(&m_memory);
		m_audio.configureMemory(&m_memory);
		m_joypad.configureMemory(&m_memory);
		m_cart.configureMemory(&m_memory);
		m_serial.configureMemory(&m_memory);
		m_dma.configureMemory(&m_memory);
		m_memory.build();

		//std::cout << oh8(m_memory.get(0x0144)) << " " << oh8(m_memory.get(0xFF40)) << " " << oh8(m_memory.get(0x0180)) << std::endl;
	}

	void Gameboy::main() {
		std::thread uiThread(&runInterface, &m_interface, &m_lcd, &m_audio, &m_joypad);

		GBComponent cpuComponent = m_cpu.run(&m_memory, &m_dma);
		GBComponent lcdComponent = m_lcd.run();
		GBComponent dmaComponent = m_dma.run(&m_memory);
		GBComponent audioComponent = m_audio.run();

		clocktime_t startTime = std::chrono::steady_clock::now();
		int64_t lastCycle = 0;

		/*uint64_t cycleCount = 0;
		clocktime_t cycleStart = std::chrono::steady_clock::now();*/
		while (true){
			/*cycleCount += 1;
			if (cycleCount % 0x400000 == 0){
				clocktime_t cycleEnd = std::chrono::steady_clock::now();
				double duration = std::chrono::duration_cast<std::chrono::microseconds>(cycleEnd - cycleStart).count() / 1000000.0;
				std::cout << 0x400000 << " cycles in " << duration << " seconds : " << 100.0 / duration << "%" << std::endl;
				cycleStart = cycleEnd;
			}*/

			cpuComponent.onCycle();
			lcdComponent.onCycle();
			dmaComponent.onCycle();
			audioComponent.onCycle();

			int64_t target = lastCycle + CLOCK_CYCLE_NS;
			while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - startTime).count() < target);
			lastCycle = target;
		}

		uiThread.join();
	}

	void runInterface(Interface* interface, LCDController* lcd, AudioController* audio, JoypadController* joypad){
		interface->run(lcd, audio, joypad);
	}
}
