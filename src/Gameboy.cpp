#include "Gameboy.hpp"
#include <iostream>

#define MONITOR_SPEED

namespace toygb {
	// Wait for the given time (in nanoseconds) from the given start time point
	// This implements some kind of "semi-busy" wait, where we bruteforce the delay to skip,
	// but yield to other threads in-between measurements
	// The actual skipped delay is thus always a bit more than requested, but it is reasonably accurate and is accounted for anyway
	static inline int64_t waitFor(clocktime_t start, int64_t nanoseconds) {
		int64_t delay;
		do {
			std::this_thread::yield();
			delay = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
		} while (delay < nanoseconds);
		return delay;
	}

	// Initialize the emulator
	Gameboy::Gameboy(GameboyConfig& config):
		m_config(config), m_cpu(config), m_lcd(),
		m_interrupt(), m_audio(), m_cart(), m_serial(), m_dma(),
		m_hardware(config.mode, config.console, config.system) {
	}

	// Start the emulator
	void Gameboy::main() {
		// Load the ROM and save files
		m_cart.init(m_config.romfile, m_config.ramfile, &m_hardware);

		// Configure the hardware, issuing warnings (just in case) if the user set a hardware configuration different than the preferred one for the cartridge
		HardwareStatus cartConfig = m_cart.getDefaultHardwareStatus();
		if (m_hardware.mode() == OperationMode::Auto)
			m_hardware.setOperationMode(cartConfig.mode());
		else if (m_hardware.mode() != cartConfig.mode() && cartConfig.mode() != OperationMode::Auto)
			std::cerr << "Override operation mode : default was " << std::to_string(cartConfig.mode()) << ", set " << std::to_string(m_hardware.mode()) << std::endl;;

		if (m_hardware.console() == ConsoleModel::Auto)
			m_hardware.setConsoleModel(cartConfig.console());
		else if (m_hardware.console() != cartConfig.console() && cartConfig.console() != ConsoleModel::Auto)
			std::cerr << "Override console model : default was " << std::to_string(cartConfig.console()) << ", set " << std::to_string(m_hardware.console()) << std::endl;

		if (m_hardware.system() == SystemRevision::Auto)
			m_hardware.setSystemRevision(cartConfig.system());
		else if (m_hardware.system() != cartConfig.system() && cartConfig.system() != SystemRevision::Auto)
			std::cerr << "Override console model : default was " << std::to_string(cartConfig.system()) << ", set " << std::to_string(m_hardware.system()) << std::endl;

		m_hardware.setAutoConfig();

		std::cout << "Hardware config : mode " << std::to_string(m_hardware.mode()) << ", console " << std::to_string(m_hardware.console()) << ", system " << std::to_string(m_hardware.system()) << std::endl;

		// Initialize components
		m_cpu.init(&m_hardware, &m_interrupt);  // Must initialize the CPU first as it checks the bootrom status

		if (m_hardware.hasBootrom() && m_hardware.isCGBCapable())
			m_hardware.setOperationMode(OperationMode::CGB);

		m_interrupt.init();
		m_hardware.init(&m_interrupt);
		m_lcd.init(&m_hardware, &m_interrupt);
		m_audio.init(&m_hardware);
		m_joypad.init(&m_hardware, &m_interrupt);
		m_serial.init(&m_hardware, &m_interrupt);
		m_dma.init(&m_hardware);

		// Build the memory map
		m_memory = MemoryMap();
		m_interrupt.configureMemory(&m_memory);
		m_hardware.configureMemory(&m_memory);
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

		uint64_t cycleCount = 0;
#ifdef MONITOR_SPEED
		int cycleDelay = 0;
		clocktime_t cycleStart = std::chrono::steady_clock::now();
#endif

		clocktime_t blockStart = std::chrono::steady_clock::now();
		int64_t inaccuracyReserve = 0;
		while (!m_interface.isStopping()) {
			// Run a clock cycle. FIXME : the order of the components here is dictated by emulator behaviour technicalities, is it significant ?
			// Currently, CPU must be before DMA because of OAM DMA startup cycles handling
			//            CPU must be before APU because that’s how we manage wave RAM access, but it could be done the other way by changing AudioWaveMapping::start
			// The skip() methods here allow a little optimisation by not triggering a coroutine resume (context commutation) if it is useless (e.g the component is turned off)
			m_hardware.update();
			int sequencer = m_hardware.getSequencer();

			if ((sequencer & 0b11) == 0 && !m_hardware.isStopped()) {
				if (!m_cpu.skip())
					cpuComponent.onCycle();
				m_dma.runCycle();
			}
			// Exit STOP mode when a selected joypad button is pressed (when a bit goes low)
			else if ((m_memory.get(IO_JOYPAD) & 0x0F) < 0x0F) {
				m_hardware.setStopMode(false);
			}

			// Keep the same timing for the APU and PPU, even in double-speed mode
			if (!m_lcd.skip() && ((sequencer & 0b01) == 0 || !m_hardware.doubleSpeed()))
				lcdComponent.onCycle();
			if ((sequencer & (m_hardware.doubleSpeed() ? 0b11 : 0b01)) == 0)
				m_audio.runCycle();

			m_cart.update();

			// Wait to skip excess time in-between cycles
			// The timers are not accurate up to the nanosecond and it would be terribly inefficient to busy wait at each cycle for a few nanoseconds
			// Thus we run cycles by "blocks", and "semi-busy wait" (see waitFor) during the excess time between each block
			cycleCount += 1;
			if (cycleCount % BLOCK_CYCLES == 0) {
				clocktime_t blockEnd = std::chrono::steady_clock::now();
				int64_t blockNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(blockEnd - blockStart).count();
				blockStart = blockEnd;  // Must set this as soon as possible for better accuracy
				int64_t expectedNanoseconds = int(BLOCK_CYCLES * (m_hardware.doubleSpeed() ? DOUBLESPEED_CLOCK_CYCLE_NS_REAL : CLOCK_CYCLE_NS_REAL));
				inaccuracyReserve += expectedNanoseconds - blockNanoseconds;
				// inaccuracyReserve is the current excess time, we only wait when it reaches a certain threshold for better efficiency
				if (inaccuracyReserve >= MIN_WAIT_TIME_NS) {
					int64_t actualDelay = waitFor(blockEnd, inaccuracyReserve);  // Give it blockEnd as a start to account for everything that happened since its measurement
					blockStart = std::chrono::steady_clock::now();  // Set this as soon as possible
#ifdef MONITOR_SPEED
					cycleDelay += actualDelay;
#endif
					inaccuracyReserve -= actualDelay;  // The actual delay we waited is always a bit more, so the inaccuracyReserve can be negative to account for it
				}
			}

#ifdef MONITOR_SPEED
			if (cycleCount % 0x400000 == 0) {
				clocktime_t cycleEnd = std::chrono::steady_clock::now();
				double duration = std::chrono::duration_cast<std::chrono::microseconds>(cycleEnd - cycleStart).count() / 1000000.0;
				std::cout << 0x400000 << " cycles in " << duration << " seconds : " << 100.0 / duration << "% (" << int(0x400000 / duration) << " Hz), " << cycleDelay / 1000000000.0 << "s of delays (" << cycleDelay / (duration*10000000.0) << "%)" << std::endl;
				cycleStart = cycleEnd;
				cycleDelay = 0.0;
			}
#endif
		}

		uiThread.join();
		m_cart.save();  // TODO : Currently only saves at exit, add autosaves in case of crash or whatever ?
	}

	void runInterface(Interface* interface, LCDController* lcd, AudioController* audio, JoypadController* joypad) {
		interface->run(lcd, audio, joypad);
	}
}
