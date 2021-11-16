#ifndef _CORE_CPU_HPP
#define _CORE_CPU_HPP

#include "util/component.hpp"
#include "core/timing.hpp"
#include "core/InterruptVector.hpp"
#include "core/OperationMode.hpp"
#include "memory/MemoryMapping.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/Constants.hpp"
#include "memory/DMAController.hpp"

#include "memory/mapping/ArrayMemoryMapping.hpp"
#include "memory/mapping/TimerMapping.hpp"
#include "memory/mapping/FixBankedMemoryMapping.hpp"
#include "memory/mapping/WRAMBankSelectMapping.hpp"


namespace toygb {
	class CPU {
		public:
			CPU();
			CPU(bool disassemble);
			~CPU();

			void configureMemory(MemoryMap* memory);
			void init(OperationMode mode, InterruptVector* interrupt);

			GBComponent run(MemoryMap* memory, DMAController* dma);

		private:
			void initRegisters();
			uint8_t memoryRead(uint16_t address);
			void memoryWrite(uint16_t address, uint8_t value);
			bool checkCondition(uint8_t condition);
			void setFlags(uint8_t z, uint8_t n, uint8_t h, uint8_t c);
			void accumulatorOperation(uint8_t operation, uint8_t operand);
			void increment16(uint8_t* high, uint8_t* low);
			void decrement16(uint8_t* high, uint8_t* low);
			void incrementTimer(int cycles);
			void applyDAA();

			void logDisassembly(uint16_t position);
			void logStatus();


			MemoryMap* m_memory;
			DMAController* m_dma;
			InterruptVector* m_interrupt;
			OperationMode m_mode;

			uint8_t* m_wram;
			uint8_t* m_hram;

			MemoryMapping* m_hramMapping;
			MemoryMapping* m_wramMapping;
			TimerMapping* m_timer;
			WRAMBankSelectMapping* m_wramBankMapping;

			uint8_t m_wramBank;

			uint8_t m_registers[8];
			uint16_t m_sp;
			uint16_t m_pc;
			bool m_ei_scheduled;
			int64_t m_instructionCount;

			bool m_halted;
			bool m_stopped;
			bool m_haltBug;

			int m_timaCounter;
			int m_dividerCounter;

			bool m_logDisassembly;
	};
}

#endif
