#ifndef _CORE_CPU_HPP
#define _CORE_CPU_HPP

#include "core/timing.hpp"
#include "core/hardware.hpp"
#include "core/InterruptVector.hpp"
#include "core/mapping/TimerMapping.hpp"
#include "core/mapping/WRAMBankSelectMapping.hpp"
#include "memory/MemoryMapping.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/Constants.hpp"
#include "memory/DMAController.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"
#include "memory/mapping/FixBankedMemoryMapping.hpp"
#include "util/component.hpp"


namespace toygb {
	/** Implements the Gameboy's Sharp LR35902 (z80-like) CPU */
	class CPU {
		public:
			CPU();
			CPU(bool disassemble);  // TODO : Replace this with some proper debug tools and configuration
			~CPU();

			void configureMemory(MemoryMap* memory);
			void init(HardwareConfig& hardware, InterruptVector* interrupt);

			/** Main CPU loop, as a coroutine */
			GBComponent run(MemoryMap* memory, DMAController* dma);

			/** Tell whether the emulator can skip the component on that cycle, to save a context commutation */
			bool skip();

		private:
			// General utilities
			void initRegisters();                                            // Initialize the register with the hardware's defaut values (as much as possible). TODO : Add bootroms
			uint8_t memoryRead(uint16_t address);                            // Get the value at the given memory address
			void memoryWrite(uint16_t address, uint8_t value);               // Set the value at the given memory address

			// Instruction execution helpers
			bool checkCondition(uint8_t condition);                          // Evaluate a conditional instruction's condition (as specified by the condition identifier in the opcode z, c, nz, nc)
			void setFlags(uint8_t z, uint8_t n, uint8_t h, uint8_t c);       // Set the flags to the given value, or UNAFFECTED
			void accumulatorOperation(uint8_t operation, uint8_t operand);   // Perform an arithmetical operation (as specified by the operation identifier in the opcode) on the accumulator, with the given operand
			void increment16(uint8_t* high, uint8_t* low);                   // Increment a 16-bits coupled register (high is the higher byte register, low the lower byte register)
			void decrement16(uint8_t* high, uint8_t* low);                   // Decrement a 16-bits coupled register
			uint16_t get16(uint8_t identifier);                                  // Get the value of a 16-bits register, as specified by the identifier in the opcode
			void set16(uint8_t identifier, uint16_t value);                  // Set the value of a 16-bits register, as specified by the identifier in the opcode
			void set16(uint8_t identifier, uint8_t high, uint8_t low);       // Set the value of a 16-bits register, as specified by the identifier in the opcode, with the higher and lower bytes of the 16-bits value
			void incrementTimer(int cycles);                                 // Increment the timer registers
			void applyDAA();                                                 // Execute the DAA (Decimally Adjust the Accumulator)

			// Debug thingies. TODO : Replace this with proper tools
			void logDisassembly(uint16_t position);
			void logStatus();


			MemoryMap* m_memory;           // Global memory map, as accessible to the CPU
			DMAController* m_dma;          // The global DMA controller, for memory access shenanigans
			InterruptVector* m_interrupt;  // Global interrupt vector
			HardwareConfig m_hardware;

			uint8_t* m_wram;  // WRAM data array (mapped on 0xC000-0xDFFF)
			uint8_t* m_hram;  // HRAM data array (mapped on 0xFF80-0xFFFE)

			MemoryMapping* m_hramMapping;              // HRAM memory mapping
			MemoryMapping* m_wramMapping;              // WRAM memory mapping
			TimerMapping* m_timer;                     // Timer IO registers memory mapping
			WRAMBankSelectMapping* m_wramBankMapping;  // For CGB mode, WRAM bank select IO register (register SVBK, 0xFF70)

			uint8_t m_wramBank;  // WRAM bank that is currently mapped

			uint8_t m_registers[8];      // Main CPU registers, index is the register identifier in opcodes [b, c, d, e, h, l, f, a]
			uint16_t m_sp;               // Stack pointer register
			uint16_t m_pc;               // Program counter
			bool m_ei_scheduled;         // Whether a EI (enable interrupts) instruction has been run on the last cycle, to respect the 1 cycle delay before activation
			int64_t m_instructionCount;  // Counts the instructions that have been executed, for debug. TODO : Remove this

			bool m_halted;   // HALT status (set by the halt instruction)
			bool m_stopped;  // STOP status (set by the stop instruction)
			bool m_haltBug;  // Whether the halt instruction was just used in a situation that causes the program counter to not be incremented

			int m_timaCounter;     // Counts cycles for the TIMA register timer
			int m_dividerCounter;  // Counts cycles for the DIV register timer

			bool m_logDisassembly;

			int m_cyclesToSkip;
	};
}

#endif
