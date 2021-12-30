#include "Gameboy.hpp"
#include <iostream>

namespace toygb {
	// Initialize the emulator
	Gameboy::Gameboy(GameboyConfig& config):
		m_config(config), m_cpu(config), m_lcd(),
		m_interrupt(), m_audio(), m_cart(), m_serial(), m_dma(),
		m_hardware(config.mode, config.console, config.system) {
	}

	// Start the emulator
	void Gameboy::main() {
		// Load the ROM and save files
		m_cart.init(m_config.romfile, m_config.ramfile);

		// Configure the hardware, issuing warnings (just in case) if the user set a hardware configuration different than the preferred one for the cartridge
		HardwareConfig cartConfig = m_cart.getDefaultHardwareConfig();
		if (m_hardware.mode() == OperationMode::Auto)
			m_hardware.setOperationMode(cartConfig.mode());
		else if (m_hardware.mode() != cartConfig.mode())
			std::cerr << "Override operation mode : default was " << std::to_string(cartConfig.mode()) << ", set " << std::to_string(m_hardware.mode());

		if (m_hardware.console() == ConsoleModel::Auto)
			m_hardware.setConsoleModel(cartConfig.console());
		else if (m_hardware.console() != cartConfig.console())
			std::cerr << "Override console model : default was " << std::to_string(cartConfig.console()) << ", set " << std::to_string(m_hardware.console());

		if (m_hardware.system() == SystemRevision::Auto)
			m_hardware.setSystemRevision(cartConfig.system());
		else if (m_hardware.system() != cartConfig.system())
			std::cerr << "Override console model : default was " << std::to_string(cartConfig.system()) << ", set " << std::to_string(m_hardware.system());

		m_hardware.setAutoConfig();

		// Initialize components
		m_cpu.init(&m_hardware, &m_interrupt);  // Must initialize the CPU first as it checks the bootrom status

		if (m_hardware.hasBootrom() && m_hardware.isCGBCapable())
			m_hardware.setOperationMode(OperationMode::CGB);

		m_interrupt.init();
		m_lcd.init(&m_hardware, &m_interrupt);
		m_audio.init(&m_hardware);
		m_joypad.init(&m_hardware, &m_interrupt);
		m_serial.init(&m_hardware, &m_interrupt);
		m_dma.init(&m_hardware);

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

		// Start the interface
		std::thread uiThread(&runInterface, &m_interface, &m_lcd, &m_audio, &m_joypad);

		// Start the clocked components
		GBComponent cpuComponent = m_cpu.run(&m_memory, &m_dma);
		GBComponent lcdComponent = m_lcd.run();
		GBComponent dmaComponent = m_dma.run(&m_memory);
		GBComponent audioComponent = m_audio.run();

		clocktime_t startTime = std::chrono::steady_clock::now();
		double lastCycle = 0;

		/*uint64_t cycleCount = 0, cycleDelays = 0;
		clocktime_t cycleStart = std::chrono::steady_clock::now();*/
		while (!m_interface.isStopping()) {
			// Run a cycle. FIXME : the order of the components here is arbitrary, is it significant ?
			// The skip() methods here allow a little optimisation by not triggering a coroutine resume (context commutation) if it is useless (e.g the component is turned off)
			if (!m_cpu.skip()) cpuComponent.onCycle();
			if (!m_lcd.skip()) lcdComponent.onCycle();
			if (!m_dma.skip()) dmaComponent.onCycle();
			if (!m_audio.skip()) audioComponent.onCycle();

			// Wait till the next cycle
			// Here we use a floating point number for the period in nanoseconds, obviously the timer is not precise to the picosecond so it's not perfect at every cycle,
			// but it compensates in the long run, truncating this value into an integer would constantly make the emulator run a little bit too fast (~100.54% speed)
			double target = lastCycle + CLOCK_CYCLE_NS_REAL;
			while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - startTime).count() < target)/*
				cycleDelays += 1*/;
			lastCycle = target;

			/*cycleCount += 1;
			if (cycleCount % 0x400000 == 0){
				clocktime_t cycleEnd = std::chrono::steady_clock::now();
				double duration = std::chrono::duration_cast<std::chrono::microseconds>(cycleEnd - cycleStart).count() / 1000000.0;
				std::cout << 0x400000 << " cycles in " << duration << " seconds : " << 100.0 / duration << "% (" << int(0x400000 / duration) << " Hz), " << cycleDelays << " delays (avg. " << (double(cycleDelays) / 0x400000) << " delays/cycle)" << std::endl;
				cycleStart = cycleEnd;
				cycleDelays = 0;
			}*/
		}

		uiThread.join();
		m_cart.save();  // TODO : Currently only saves at exit, add autosaves in case of crash or whatever ?
	}

	void runInterface(Interface* interface, LCDController* lcd, AudioController* audio, JoypadController* joypad) {
		interface->run(lcd, audio, joypad);
	}
}
