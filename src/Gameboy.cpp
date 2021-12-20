#include "Gameboy.hpp"
#include <iostream>

namespace toygb {
	Gameboy::Gameboy(GameboyConfig& config):
		m_config(config), m_cpu(config.disassemble), m_lcd(),
		m_interrupt(), m_audio(), m_cart(), m_serial(), m_dma(),
		m_hardware(config.mode, config.console, config.system) {
	}

	void Gameboy::main() {
		m_cart.init(m_config.romfile, m_config.ramfile);

		HardwareConfig cartConfig = m_cart.getDefaultHardwareConfig();
		if (m_hardware.mode() != cartConfig.mode() && m_hardware.mode() != OperationMode::Auto)
			std::cerr << "Override operation mode : default was " << std::to_string(cartConfig.mode()) << ", set " << std::to_string(m_hardware.mode());
		if (m_hardware.mode() == OperationMode::Auto)
			m_hardware.setOperationMode(cartConfig.mode());

		if (m_hardware.console() != cartConfig.console() && m_hardware.console() != ConsoleModel::Auto)
			std::cerr << "Override console model : default was " << std::to_string(cartConfig.console()) << ", set " << std::to_string(m_hardware.console());
		if (m_hardware.console() == ConsoleModel::Auto)
			m_hardware.setConsoleModel(cartConfig.console());

		if (m_hardware.system() != cartConfig.system() && m_hardware.system() != SystemRevision::Auto)
			std::cerr << "Override console model : default was " << std::to_string(cartConfig.system()) << ", set " << std::to_string(m_hardware.system());
		if (m_hardware.system() == SystemRevision::Auto)
			m_hardware.setSystemRevision(cartConfig.system());

		// Initialize components
		m_interrupt.init(m_hardware);
		m_cpu.init(m_hardware, &m_interrupt);
		m_lcd.init(m_hardware, &m_interrupt);
		m_audio.init(m_hardware);
		m_joypad.init(m_hardware, &m_interrupt);
		m_serial.init(m_hardware, &m_interrupt);
		m_dma.init(m_hardware);

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

		std::thread uiThread(&runInterface, &m_interface, &m_lcd, &m_audio, &m_joypad);

		GBComponent cpuComponent = m_cpu.run(&m_memory, &m_dma);
		GBComponent lcdComponent = m_lcd.run();
		GBComponent dmaComponent = m_dma.run(&m_memory);
		GBComponent audioComponent = m_audio.run();

		clocktime_t startTime = std::chrono::steady_clock::now();
		double lastCycle = 0;

		/*uint64_t cycleCount = 0;
		clocktime_t cycleStart = std::chrono::steady_clock::now();*/
		while (!m_interface.isStopping()){
			cpuComponent.onCycle();
			lcdComponent.onCycle();
			dmaComponent.onCycle();
			audioComponent.onCycle();

			double target = lastCycle + CLOCK_CYCLE_NS_REAL;
			while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - startTime).count() < target);
			lastCycle = target;

			/*cycleCount += 1;
			if (cycleCount % 0x400000 == 0){
				clocktime_t cycleEnd = std::chrono::steady_clock::now();
				double duration = std::chrono::duration_cast<std::chrono::microseconds>(cycleEnd - cycleStart).count() / 1000000.0;
				std::cout << 0x400000 << " cycles in " << duration << " seconds : " << 100.0 / duration << "% (" << int(0x400000 / duration) << " Hz)" << std::endl;
				cycleStart = cycleEnd;
			}*/
		}

		uiThread.join();
		m_cart.save();
	}

	void runInterface(Interface* interface, LCDController* lcd, AudioController* audio, JoypadController* joypad){
		interface->run(lcd, audio, joypad);
	}
}
