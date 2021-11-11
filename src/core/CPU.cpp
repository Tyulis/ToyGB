#include "core/CPU.hpp"

#define reg_b m_registers[0]
#define reg_c m_registers[1]
#define reg_d m_registers[2]
#define reg_e m_registers[3]
#define reg_h m_registers[4]
#define reg_l m_registers[5]
#define reg_f m_registers[6]
#define reg_a m_registers[7]

#define reg_bc ((reg_b << 8) | reg_c)
#define reg_de ((reg_d << 8) | reg_e)
#define reg_hl ((reg_h << 8) | reg_l)
#define reg_af ((reg_a << 8) | reg_f)
#define reg_sp m_sp

#define flag_z ((reg_f >> 7) & 1)
#define flag_n ((reg_f >> 6) & 1)
#define flag_h ((reg_f >> 5) & 1)
#define flag_c ((reg_f >> 4) & 1)

#define mask_flag_z 0b10000000
#define mask_flag_n 0b01000000
#define mask_flag_h 0b00100000
#define mask_flag_c 0b00010000

#define UNAFFECTED 0xFF


namespace toygb {
	CPU::CPU(){
		m_hram = nullptr;
		m_wram = nullptr;

		m_timer = nullptr;
		m_wramMapping = nullptr;
		m_hramMapping = nullptr;
	}

	CPU::~CPU(){
		if (m_hram != nullptr) delete[] m_hram;
		if (m_wram != nullptr) delete[] m_wram;

		if (m_hramMapping != nullptr) delete m_hramMapping;
		if (m_wramMapping != nullptr) delete m_wramMapping;
		if (m_timer != nullptr) delete m_timer;
	}

	void CPU::init(OperationMode mode, InterruptVector* interrupt){
		m_mode = mode;
		m_interrupt = interrupt;
		m_hram = new uint8_t[HRAM_SIZE];

		if (mode == OperationMode::DMG){
			m_wram = new uint8_t[WRAM_SIZE];
		} else if (mode == OperationMode::CGB) {
			m_wram = new uint8_t[WRAM_BANK_SIZE * WRAM_BANK_NUM];
		}
	}

	void CPU::configureMemory(MemoryMap* memory) {
		// HRAM
		m_hramMapping = new ArrayMemoryMapping(m_hram);
		memory->add(HRAM_OFFSET, HRAM_OFFSET + HRAM_SIZE - 1, m_hramMapping);

		// WRAM
		if (m_mode == OperationMode::DMG){
			m_wramMapping = new ArrayMemoryMapping(m_wram);
		} else if (m_mode == OperationMode::CGB) {
			m_wramBankMapping = new WRAMBankSelectMapping(&m_wramBank);
			m_wramMapping = new BankedWRAMMapping(&m_wramBank, WRAM_BANK_SIZE, m_wram);

			memory->add(IO_WRAM_BANK, IO_WRAM_BANK, m_wramBankMapping);
		}
		memory->add(WRAM_OFFSET, WRAM_OFFSET + WRAM_SIZE - 1, m_wramMapping);
		memory->add(ECHO_OFFSET, ECHO_OFFSET + ECHO_SIZE - 1, m_wramMapping);

		// Timer IO
		m_timer = new TimerMapping();
		memory->add(IO_TIMER_DIVIDER, IO_TIMER_CONTROL, m_timer);
	}

#define cycle() co_await std::suspend_always(); co_await std::suspend_always(); co_await std::suspend_always(); co_await std::suspend_always();

	GBComponent CPU::run(MemoryMap* memory){
		m_memory = memory;
		initRegisters();

		// Start CPU operation
		uint8_t opcode = m_memory->get(m_pc++);  // Start with first opcode already fetched

		while (true){
			if (m_ei_scheduled){
				m_ei_scheduled = false;
				m_interrupt->setMaster(true);
			}

			Interrupt interrupt = m_interrupt->getInterrupt();
			if (interrupt != Interrupt::None){
				if (m_halted){
					m_halted = false;
					opcode = memoryRead(m_pc++); cycle();
				}

				// Jump to interrupt vector only when IME is set
				if (m_interrupt->getMaster()){
					cycle(); cycle();
					m_interrupt->resetRequest(interrupt);
					m_interrupt->setMaster(false);
					m_sp -= 1;
					memoryWrite(m_sp--, (m_pc - 1) >> 8); cycle();
					memoryWrite(m_sp, (m_pc - 1) & 0xFF); cycle();

					switch (interrupt){
						case Interrupt::VBlank:  m_pc = 0x0040; break;
						case Interrupt::LCDStat: m_pc = 0x0048; break;
						case Interrupt::Timer:   m_pc = 0x0050; break;
						case Interrupt::Serial:  m_pc = 0x0058; break;
						case Interrupt::Joypad:  m_pc = 0x0060; break;
						case Interrupt::None: break;
					}
					opcode = memoryRead(m_pc++); cycle();
				}
				continue;
			}

			uint16_t basePC = m_pc - 1;
			m_instructionCount += 1;

			// 00 opcodes
			if (opcode == 0b00000000){  // 00 000000 = nop
				// nop
			} else if (opcode == 0b00010000){  // 00 01 0000 = stop
				/*uint8_t value =*/ memoryRead(m_pc++); cycle(); // TODO : Invalid stop values ?
				// TODO
			} else if ((opcode & 0b11100111) == 0b00100000){  // 001 cc 000 = jr cc, e
				uint8_t condition = (opcode >> 3) & 3;
				int8_t diff = int8_t(memoryRead(m_pc++)); cycle();
				if (checkCondition(condition)){
					m_pc += diff;
					cycle();
				}
			} else if (opcode == 0b00000001){  // 00 00 0001 = ld bc, nn
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				reg_b = high; reg_c = low;
			} else if (opcode == 0b00010001){  // 00 01 0001 = ld de, nn
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				reg_d = high; reg_e = low;
			} else if (opcode == 0b00100001){  // 00 10 0001 = ld hl, nn
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				reg_h = high; reg_l = low;
			} else if (opcode == 0b00110001){  // 00 11 0001 = ld sp, nn
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				uint16_t value = (high << 8) | low; cycle();
				m_sp = value;
			} else if (opcode == 0b00000010){  // 00 00 0010 = ld (bc), a
				memoryWrite(reg_bc, reg_a); cycle();
			} else if (opcode == 0b00010010){  // 00 01 0010 = ld (de), a
				memoryWrite(reg_de, reg_a); cycle();
			} else if (opcode == 0b00100010){  // 00 10 0010 = ldi (hl), a
				memoryWrite(reg_hl, reg_a); cycle();
				increment16(&reg_h, &reg_l); cycle();
			} else if (opcode == 0b00110010){  // 00 11 0010 = ldd (hl), a
				memoryWrite(reg_hl, reg_a); cycle();
				decrement16(&reg_h, &reg_l); cycle();
			} else if (opcode == 0b00000011){  // 00 00 0011 = inc bc
				cycle();
				increment16(&reg_b, &reg_c);
			} else if (opcode == 0b00010011){  // 00 01 0011 = inc de
				cycle();
				increment16(&reg_d, &reg_e);
			} else if (opcode == 0b00100011){  // 00 10 0011 = inc hl
				cycle();
				increment16(&reg_h, &reg_l);
			} else if (opcode == 0b00110011){  // 00 11 0011 = inc sp
				cycle();
				m_sp += 1;
			} else if (opcode == 0b00110100){  // 00 110 100 = inc (hl)
				uint8_t value = memoryRead(reg_hl); cycle();
				uint8_t result = value + 1;
				memoryWrite(reg_hl, result); cycle();
				setFlags(result == 0, 0, (result & 0x0F) < (value & 0x0F), UNAFFECTED);
			} else if ((opcode & 0b11000111) == 0b00000100){  // 00 rrr 100 = inc r
				uint8_t reg = (opcode >> 3) & 7;
				uint8_t value = m_registers[reg];
				uint8_t result = value + 1;
				m_registers[reg] = result;
				setFlags(result == 0, 0, (result & 0x0F) < (value & 0x0F), UNAFFECTED);
			} else if (opcode == 0b00110101){  // 00 110 101 = dec (hl)
				uint8_t value = memoryRead(reg_hl); cycle();
				uint8_t result = value - 1;
				memoryWrite(reg_hl, result); cycle();
				setFlags(result == 0, 1, (result & 0x0F) > (value & 0x0F), UNAFFECTED);
			} else if ((opcode & 0b11000111) == 0b00000101){  // 00 rrr 101 = dec r
				uint8_t reg = (opcode >> 3) & 7;
				uint8_t value = m_registers[reg];
				uint8_t result = value - 1;
				m_registers[reg] = result;
				setFlags(result == 0, 1, (result & 0x0F) > (value & 0x0F), UNAFFECTED);
			} else if (opcode == 0b00110110){  // 00 110 110 = ld (hl), n
				uint8_t value = memoryRead(m_pc++); cycle();
				memoryWrite(reg_hl, value); cycle();
			} else if ((opcode & 0b11000111) == 0b00000110){  // 00 rrr 110 = ld r, n
				uint8_t value = memoryRead(m_pc++); cycle();
				uint8_t destreg = (opcode >> 3) & 7;
				m_registers[destreg] = value;
			} else if (opcode == 0b00000111){  // 00 00 0111 = rlca
				reg_a = (reg_a << 1) | (reg_a >> 7);
				setFlags(0, 0, 0, reg_a & 1);
			} else if (opcode == 0b00010111){  // 00 01 0111 = rla
				bool newcarry = reg_a >> 7;
				reg_a = (reg_a << 1) | flag_c;
				setFlags(0, 0, 0, newcarry);
			} else if (opcode == 0b00100111){  // 00 10 0111 = daa
				// TODO
			} else if (opcode == 0b00110111){  // 00 11 0111 = scf
				reg_f |= mask_flag_c;  // set carry
				reg_f &= ~(mask_flag_n | mask_flag_h);  // clear n and h flags
			} else if (opcode == 0b00001000){  // 00 00 1000 = ld (nn), sp
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				uint16_t address = (high << 8) | low;
				memoryWrite(address, m_sp & 0xFF); cycle();
				memoryWrite(address + 1, m_sp >> 8); cycle();
			} else if (opcode == 0b00011000){  // 00 01 1000 = jr e
				int8_t diff = int8_t(memoryRead(m_pc++)); cycle();
				m_pc += diff;
				cycle();
			} else if (opcode == 0b00001001){  // 00 00 1001 = add hl, bc
				uint16_t result = reg_hl + reg_bc; cycle();
				setFlags(UNAFFECTED, 0, (result & 0x0FFF) < (reg_hl & 0x0FFF), result < reg_hl);
				reg_h = result >> 8;
				reg_l = result & 0xFF;
			} else if (opcode == 0b00011001){  // 00 01 1001 = add hl, de
				uint16_t result = reg_hl + reg_de; cycle();
				setFlags(UNAFFECTED, 0, (result & 0x0FFF) < (reg_hl & 0x0FFF), result < reg_hl);
				reg_h = result >> 8;
				reg_l = result & 0xFF;
			} else if (opcode == 0b00101001){  // 00 10 1001 = add hl, hl
				uint16_t result = reg_hl + reg_hl; cycle();
				setFlags(UNAFFECTED, 0, (result & 0x0FFF) < (reg_hl & 0x0FFF), result < reg_hl);
				reg_h = result >> 8;
				reg_l = result & 0xFF;
			} else if (opcode == 0b00111001){  // 00 11 1001 = add hl, sp
				uint16_t result = reg_hl + m_sp; cycle();
				setFlags(UNAFFECTED, 0, (result & 0x0FFF) < (reg_hl & 0x0FFF), result < reg_hl);
				reg_h = result >> 8;
				reg_l = result & 0xFF;
			} else if (opcode == 0b00001010){  // 00 00 1010 = ld a, (bc)
				uint8_t value = memoryRead(reg_bc); cycle();
				reg_a = value;
			} else if (opcode == 0b00011010){  // 00 01 1010 = ld a, (de)
				uint8_t value = memoryRead(reg_de); cycle();
				reg_a = value;
			} else if (opcode == 0b00101010){  // 00 10 1010 = ldi a, (hl)
				uint8_t value = memoryRead(reg_hl); cycle();
				increment16(&reg_h, &reg_l);
				reg_a = value;
			} else if (opcode == 0b00111010){  // 00 11 1010 = ldd a, (hl)
				uint8_t value = memoryRead(reg_hl); cycle();
				decrement16(&reg_h, &reg_l);
				reg_a = value;
			} else if (opcode == 0b00001011){  // 00 00 1011 = dec bc
				cycle();
				decrement16(&reg_b, &reg_c);
			} else if (opcode == 0b00011011){  // 00 01 1011 = dec de
				cycle();
				decrement16(&reg_d, &reg_e);
			} else if (opcode == 0b00101011){  // 00 10 1011 = dec hl
				cycle();
				decrement16(&reg_h, &reg_l);
			} else if (opcode == 0b00111011){  // 00 11 1011 = dec sp
				cycle();
				m_sp -= 1;
			} else if (opcode == 0b00001111){  // 00 00 1111 = rrca
				reg_a = (reg_a >> 1) | (reg_a << 7);
				setFlags(0, 0, 0, reg_a >> 7);
			} else if (opcode == 0b00011111){  // 00 01 1111 = rra
				bool newcarry = reg_a & 1;
				reg_a = (reg_a >> 1) | (flag_c << 7);
				setFlags(0, 0, 0, newcarry);
			} else if (opcode == 0b00101111){  // 00 10 1111 = cpl
				reg_a = ~reg_a;  // flip A
				setFlags(UNAFFECTED, 1, 1, UNAFFECTED);
			} else if (opcode == 0b00111111){  // 00 11 1111 = ccf
				reg_f ^= mask_flag_c;  // flip carry
				setFlags(UNAFFECTED, 0, 0, UNAFFECTED);


			// 01 opcodes
			} else if (opcode == 0b01110110){  // 01 110 110 = halt
				if (m_interrupt->getMaster()){
					m_halted = true;
				} else if (m_interrupt->getInterrupt() == Interrupt::None){
					m_halted = true;
				} else {
					// Halt bug
					m_haltBug = true;
				}
			} else if ((opcode & 0b11000111) == 0b01000110){  // 01 xxx 110 = ld x, (hl)
				uint8_t value = memoryRead(reg_hl); cycle();
				uint8_t destreg = (opcode >> 3) & 7;
				m_registers[destreg] = value;
			} else if ((opcode & 0b11111000) == 0b01110000){  // 01 110 xxx = ld (hl), x
				uint8_t sourcereg = opcode & 7;
				memoryWrite(reg_hl, m_registers[sourcereg]); cycle();
			} else if ((opcode & 0b11000000) == 0b01000000){  // 01 xxx yyy = ld x, y
				uint8_t sourcereg = opcode & 7;
				uint8_t destreg = (opcode >> 3) & 7;
				m_registers[destreg] = m_registers[sourcereg];


			// 10 opcodes
			} else if ((opcode & 0b11000111) == 0b10000110){  // 10 ooo 110 = <op> a, (hl)
				uint8_t operation = (opcode >> 3) & 7;
				uint8_t operand = memoryRead(reg_hl); cycle();
				accumulatorOperation(operation, operand);
			} else if ((opcode & 0b11000000) == 0b10000000){  // 10 ooo rrr = <op> a, r
				uint8_t operation = (opcode >> 3) & 7;
				uint8_t reg = opcode & 7;
				accumulatorOperation(operation, m_registers[reg]);


			// 11 opcodes
			} else if ((opcode & 0b11100111) == 0b11000000){  // 110 cc 000 = ret cc
				uint8_t condition = (opcode >> 3) & 3; cycle();
				if (checkCondition(condition)){
					uint16_t low = memoryRead(m_sp++); cycle();
					uint16_t high = memoryRead(m_sp++); cycle();
					uint16_t address = (high << 8) | low; cycle();
					m_pc = address;
				}
			} else if (opcode == 0b11110000){  // 11 11 0000 = ldh a, (n)
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t address = 0xFF00 | low;
				reg_a = memoryRead(address); cycle();
			} else if (opcode == 0b11100000){  // 11 10 0000 = ldh (n), a
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t address = 0xFF00 | low;
				memoryWrite(address, reg_a); cycle();
			} else if (opcode == 0b11000001){  // 11 00 0001 = pop bc
				reg_c = memoryRead(m_sp++); cycle();
				reg_b = memoryRead(m_sp++); cycle();
			} else if (opcode == 0b11010001){  // 11 01 0001 = pop de
				reg_e = memoryRead(m_sp++); cycle();
				reg_d = memoryRead(m_sp++); cycle();
			} else if (opcode == 0b11100001){  // 11 10 0001 = pop hl
				reg_l = memoryRead(m_sp++); cycle();
				reg_h = memoryRead(m_sp++); cycle();
			} else if (opcode == 0b11110001){  // 11 11 0001 = pop af
				reg_f = memoryRead(m_sp++); cycle();
				reg_a = memoryRead(m_sp++); cycle();
			} else if ((opcode & 0b11100111) == 0b11000010){  // 110 cc 010 = jp cc, nn
				uint8_t condition = (opcode >> 3) & 3;
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				uint16_t address = (high << 8) | low;
				if (checkCondition(condition)){
					cycle();
					m_pc = address;
				}
			} else if (opcode == 0b11100010){  // 11 10 0010 = ldh (c), a
				uint16_t address = 0xFF00 | reg_c;
				memoryWrite(address, reg_a); cycle();
			} else if (opcode == 0b11110010){  // 11 11 0010 = ldh a, (c)
				uint16_t address = 0xFF00 | reg_c;
				reg_a = memoryRead(address); cycle();
			} else if (opcode == 0b11000011){  // 11 00 0011 = jp nn
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				uint16_t address = (high << 8) | low;
				cycle();
				m_pc = address;
			} else if (opcode == 0b11110011){  // 11 11 0011 = di
				m_ei_scheduled = false;
				m_interrupt->setMaster(false);
			} else if ((opcode & 0b11100111) == 0b11000100){  // 110 cc 100 = call cc, nn
				uint8_t condition = (opcode >> 3) & 3;
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				if (checkCondition(condition)){
					uint16_t address = (high << 8) | low;
					m_sp -= 1; cycle();
					memoryWrite(m_sp--, m_pc >> 8); cycle();
					memoryWrite(m_sp, m_pc & 0xFF); cycle();
					m_pc = address;
				}
			} else if (opcode == 0b11000101){  // 11 00 0101 = push bc
				m_sp -= 1;
				cycle();
				memoryWrite(m_sp--, reg_b); cycle();
				memoryWrite(m_sp, reg_c); cycle();
			} else if (opcode == 0b11010101){  // 11 01 0101 = push de
				m_sp -= 1;
				cycle();
				memoryWrite(m_sp--, reg_d); cycle();
				memoryWrite(m_sp, reg_e); cycle();
			} else if (opcode == 0b11100101){  // 11 10 0101 = push hl
				m_sp -= 1;
				cycle();
				memoryWrite(m_sp--, reg_h); cycle();
				memoryWrite(m_sp, reg_l); cycle();
			} else if (opcode == 0b11110101){  // 11 11 0101 = push af
				m_sp -= 1;
				cycle();
				memoryWrite(m_sp--, reg_a); cycle();
				memoryWrite(m_sp, reg_f); cycle();
			} else if ((opcode & 0b11000111) == 0b11000110){  // 11 ooo 110 = <op> a, n
				uint8_t operation = (opcode >> 3) & 7;
				uint8_t operand = memoryRead(m_pc++); cycle();
				accumulatorOperation(operation, operand);
			} else if ((opcode & 0b11000111) == 0b11000111){  // 11 xxx 111 = rst xxx000
				uint16_t address = opcode & 0b00111000;
				m_sp -= 1; cycle();
				memoryWrite(m_sp--, m_pc >> 8); cycle();
				memoryWrite(m_sp, m_pc & 0xFF); cycle();
				m_pc = address;
			} else if (opcode == 0b11101000){  // 11 10 1000 = add sp, e = add sp, e
				uint8_t operand = int8_t(memoryRead(m_pc++)); cycle();
				uint16_t result = m_sp + operand; cycle();
				// FIXME : carry and half-carry flags for add sp, e ?
				setFlags(0, 0, (result & 0x0F00) != (m_sp & 0x0F00), (result & 0xF000) != (m_sp & 0xF000));
				m_sp = result; cycle();
			} else if (opcode == 0b11111000){  // 11 11 1000 = ld hl, sp+e
				uint8_t operand = int8_t(memoryRead(m_pc++)); cycle();
				uint16_t result = m_sp + operand; cycle();
				// FIXME : carry and half-carry flags for ld hl, sp+e ?
				setFlags(0, 0, (result & 0x0F00) != (m_sp & 0x0F00), (result & 0xF000) != (m_sp & 0xF000));
				reg_h = result >> 8;
				reg_l = result & 0xFF;
			} else if (opcode == 0b11001001){  // 11 00 1001 = ret
				uint16_t low = memoryRead(m_sp++); cycle();
				uint16_t high = memoryRead(m_sp++); cycle();
				uint16_t address = (high << 8) | low;
				m_pc = address; cycle();
			} else if (opcode == 0b11011001){  // 11 01 1001 = reti
				uint16_t low = memoryRead(m_sp++); cycle();
				uint16_t high = memoryRead(m_sp++); cycle();
				uint16_t address = (high << 8) | low;
				m_pc = address; cycle();
				m_interrupt->setMaster(true);
			} else if (opcode == 0b11101001){  // 11 10 1001 = jp hl
				m_pc = reg_hl;
			} else if (opcode == 0b11111001){  // 11 11 1001 = ld sp, hl
				cycle();
				m_sp = (reg_h << 8) | reg_l;
			} else if (opcode == 0b11101010){  // 11 10 1010 = ld (nn), a
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				uint16_t address = (high << 8) | low;
				memoryWrite(address, reg_a); cycle();
			} else if (opcode == 0b11111010){  // 11 11 1010 = ld a, (nn)
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				uint16_t address = (high << 8) | low;
				uint8_t value = memoryRead(address); cycle();
				reg_a = value;
			} else if (opcode == 0b11001011){  // 11 00 1011 = prefix CB
				uint8_t operation = memoryRead(m_pc++); cycle();
				uint8_t reg = operation & 7;

				uint8_t operand;
				if (reg == 0b110){
					operand = memoryRead(reg_hl); cycle();
				} else {
					operand = m_registers[reg];
				}

				uint8_t result = operand;
				uint8_t block = (operation >> 6) & 3;
				uint8_t subop = (operation >> 3) & 7;
				if (block == 0b00){  // 00 xxx : Bitwise operations
					if (subop == 0b000){  // 00 000 : rlc
						result = (operand << 1) | (operand >> 7);
						setFlags(result == 0, 0, 0, operand >> 7);
					} else if (subop == 0b001){  // 00 001 : rrc
						result = (operand >> 1) | (operand << 7);
						setFlags(result == 0, 0, 0, operand & 1);
					} else if (subop == 0b001){  // 00 010 : rl
						result = (operand << 1) | flag_c;
						setFlags(result == 0, 0, 0, operand >> 7);
					} else if (subop == 0b001){  // 00 011 : rr
						result = (operand >> 1) | (flag_c << 7);
						setFlags(result == 0, 0, 0, operand & 1);
					} else if (subop == 0b001){  // 00 100 : sla
						result = operand << 1;
						setFlags(result == 0, 0, 0, operand >> 7);
					} else if (subop == 0b001){  // 00 101 : sra
						result = (operand >> 1) | (operand & 0b1000000);
						setFlags(result == 0, 0, 0, operand & 1);
					} else if (subop == 0b001){  // 00 110 : swap
						result = ((operand & 0x0F) << 4) | ((operand & 0xF0) >> 4);
						setFlags(result == 0, 0, 0, 0);
					} else if (subop == 0b001){  // 00 111 : srl
						result = operand >> 1;
						setFlags(result == 0, 0, 0, operand & 1);
					}
				} else if (block == 0b01){  // 01 iii : bit i, r
					setFlags(((operand >> subop) & 1) == 0, 0, 1, UNAFFECTED);
				} else if (block == 0b10){  // 10 iii : res i, r
					result = operand & ~(1 << subop);
				} else if (block == 0b11){  // 11 iii : set i, r
					result = operand | (1 << subop);
				}


				if (reg == 0b110){
					memoryWrite(reg_hl, result); cycle();
				} else {
					m_registers[reg] = result;
				}
			} else if (opcode == 0b11111011){  // 11 11 1011 = ei
				m_ei_scheduled = true;
			} else if (opcode == 0b11001101){  // 11 00 1101 = call nn
				uint16_t low = memoryRead(m_pc++); cycle();
				uint16_t high = memoryRead(m_pc++); cycle();
				uint16_t address = (high << 8) | low;
				m_sp -= 1; cycle();
				memoryWrite(m_sp--, m_pc >> 8); cycle();
				memoryWrite(m_sp, m_pc & 0xFF); cycle();
				m_pc = address;
			}

			//logDisassembly(basePC);

			opcode = memoryRead(m_pc); cycle();  // Fetch the next opcode during the last cycle of the current instruction
			if (!m_haltBug) m_pc += 1;
		}
	}

	void CPU::initRegisters(){
		switch (m_mode){
			case OperationMode::DMG:
				reg_a = 0x01; reg_f = 0xB0;
				reg_b = 0x00; reg_c = 0x13;
				reg_d = 0x00; reg_e = 0xD8;
				reg_h = 0x01; reg_l = 0x4D;
				reg_sp = 0xFFFE;
				m_pc = 0x0100;
				break;
			case OperationMode::SGB:
				reg_a = 0x01; reg_f = 0x00;
				reg_b = 0x00; reg_c = 0x14;
				reg_d = 0x00; reg_e = 0x00;
				reg_h = 0xC0; reg_l = 0x60;
				reg_sp = 0xFFFE;
				m_pc = 0x0100;
				break;
			case OperationMode::CGB:
				reg_a = 0x11; reg_f = 0x80;
				reg_b = 0x00; reg_c = 0x00;
				reg_d = 0xFF; reg_e = 0x56;
				reg_h = 0x00; reg_l = 0x0D;
				reg_sp = 0xFFFE;
				m_pc = 0x0100;
				break;
			case OperationMode::Auto:
				throw EmulationError("OperationMode::Auto given to CPU");
		}
		m_ei_scheduled = false;
		m_haltBug = false;
		m_halted = false;
		m_instructionCount = 0;
	}

	uint8_t CPU::memoryRead(uint16_t address){
		uint8_t value = m_memory->get(address);
		return value;
	}

	void CPU::memoryWrite(uint16_t address, uint8_t value){
		m_memory->set(address, value);
	}

	bool CPU::checkCondition(uint8_t condition){
		switch (condition){
			case 0b00:  // nz
				return !flag_z;
			case 0b01:  // z
				return flag_z;
			case 0b10:  // nc
				return !flag_c;
			case 0b11:  // c
				return flag_c;
		}

		std::stringstream errstream;
		errstream << "Invalid jump condition " << int(condition);
		throw EmulationError(errstream.str());
	}

	void CPU::setFlags(uint8_t z, uint8_t n, uint8_t h, uint8_t c){
		uint8_t setmask = ((z == 1) << 7) | ((n == 1) << 6) | ((h == 1) << 5) | ((c == 1) << 4);
		uint8_t resetmask = ~(((z == 0) << 7) | ((n == 0) << 6) | ((h == 0) << 5) | ((c == 0) << 4));
		reg_f = (reg_f | setmask) & resetmask;
	}

	void CPU::accumulatorOperation(uint8_t operation, uint8_t operand){
		uint8_t result;
		switch (operation){
			case 0b000:  // add
				result = reg_a + operand;
				setFlags(result == 0, 0, ((result & 0x0F) < (reg_a & 0x0F)), (result < reg_a));
				reg_a = result;
				break;
			case 0b001:  // adc
				result = reg_a + operand + flag_c;
				setFlags(result == 0, 0, ((result & 0x0F) < (reg_a & 0x0F)), (result < reg_a));
				reg_a = result;
				break;
			case 0b010:  // sub
				result = reg_a - operand;
				setFlags(result == 0, 1, ((result & 0x0F) > (reg_a & 0x0F)), (result > reg_a));
				reg_a = result;
				break;
			case 0b011:  // sbc
				result = reg_a - operand - flag_c;
				setFlags(result == 0, 1, ((result & 0x0F) > (reg_a & 0x0F)), (result > reg_a));
				reg_a = result;
				break;
			case 0b100:  // and
				result = reg_a & operand;
				setFlags(result == 0, 0, 1, 0);
				reg_a = result;
				break;
			case 0b101:  // xor
				result = reg_a ^ operand;
				setFlags(result == 0, 0, 0, 0);
				reg_a = result;
				break;
			case 0b110:  // or
				result = reg_a | operand;
				setFlags(result == 0, 0, 0, 0);
				reg_a = result;
				break;
			case 0b111:  // cp
				result = reg_a - operand;
				setFlags(result == 0, 1, ((result & 0x0F) > (reg_a & 0x0F)), (result > reg_a));
				break;
			default:
				std::stringstream errstream;
				errstream << "Invalid 8-bit arithmetical operation " << int(operation);
				throw EmulationError(errstream.str());
				break;
		}
	}

	void CPU::cbOpcode(uint8_t operation, uint8_t reg){

	}

	// TODO : OAM glitch
	void CPU::increment16(uint8_t* high, uint8_t* low){
		uint16_t value = (*high << 8) | *low;
		value += 1;
		*high = value >> 8;
		*low = value & 0xFF;
	}

	// TODO : OAM glitch
	void CPU::decrement16(uint8_t* high, uint8_t* low){
		uint16_t value = (*high << 8) | *low;
		value -= 1;
		*high = value >> 8;
		*low = value & 0xFF;
	}


	void CPU::logDisassembly(uint16_t position){
		std::cout << m_instructionCount << " " << oh16(position) << " - ";

		uint8_t opcode = m_memory->get(position);
		uint8_t low = m_memory->get(position + 1);
		uint8_t high = m_memory->get(position + 2);
		uint16_t value = (high << 8) | low;
		std::cout << oh8(opcode) << " ";

		switch (opcode){
			case 0x00: std::cout << "\t\t" << "nop"; break;
			case 0x10: std::cout << oh8(low) << "\t\t" << "stop"; break;
			case 0x20: std::cout << oh8(low) << "\t\t" << "jr nz, " << int(int8_t(low)); break;
			case 0x30: std::cout << oh8(low) << "\t\t" << "jr nc, " << int(int8_t(low)); break;

			case 0x01: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "ld bc, $" << oh16(value); break;
			case 0x11: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "ld de, $" << oh16(value); break;
			case 0x21: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "ld hl, $" << oh16(value); break;
			case 0x31: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "ld sp, $" << oh16(value); break;

			case 0x02: std::cout << "\t\t" << "ld (bc), a"; break;
			case 0x12: std::cout << "\t\t" << "ld (de), a"; break;
			case 0x22: std::cout << "\t\t" << "ldi (hl), a"; break;
			case 0x32: std::cout << "\t\t" << "ldd (hl), a"; break;

			case 0x03: std::cout << "\t\t" << "inc bc"; break;
			case 0x13: std::cout << "\t\t" << "inc de"; break;
			case 0x23: std::cout << "\t\t" << "inc hl"; break;
			case 0x33: std::cout << "\t\t" << "inc sp"; break;

			case 0x04: std::cout << "\t\t" << "inc b"; break;
			case 0x14: std::cout << "\t\t" << "inc d"; break;
			case 0x24: std::cout << "\t\t" << "inc h"; break;
			case 0x34: std::cout << "\t\t" << "inc (hl)"; break;

			case 0x05: std::cout << "\t\t" << "dec b"; break;
			case 0x15: std::cout << "\t\t" << "dec d"; break;
			case 0x25: std::cout << "\t\t" << "dec h"; break;
			case 0x35: std::cout << "\t\t" << "dec (hl)"; break;

			case 0x06: std::cout << oh8(low) << "\t\t" << "ld b, $" << oh8(low); break;
			case 0x16: std::cout << oh8(low) << "\t\t" << "ld d, $" << oh8(low); break;
			case 0x26: std::cout << oh8(low) << "\t\t" << "ld h, $" << oh8(low); break;
			case 0x36: std::cout << oh8(low) << "\t\t" << "ld (hl), $" << oh8(low); break;

			case 0x07: std::cout << "\t\t" << "rlca"; break;
			case 0x17: std::cout << "\t\t" << "rla"; break;
			case 0x27: std::cout << "\t\t" << "daa"; break;
			case 0x37: std::cout << "\t\t" << "scf"; break;

			case 0x08: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "ld ($" << oh16(value) << "), sp"; break;
			case 0x18: std::cout << oh8(low) << "\t\t" << "jr " << int(int8_t(low)); break;
			case 0x28: std::cout << oh8(low) << "\t\t" << "jr z, " << int(int8_t(low)); break;
			case 0x38: std::cout << oh8(low) << "\t\t" << "jr c, " << int(int8_t(low)); break;

			case 0x09: std::cout << "\t\t" << "add hl, bc"; break;
			case 0x19: std::cout << "\t\t" << "add hl, de"; break;
			case 0x29: std::cout << "\t\t" << "add hl, hl"; break;
			case 0x39: std::cout << "\t\t" << "add hl, sp"; break;

			case 0x0A: std::cout << "\t\t" << "ld a, (bc)"; break;
			case 0x1A: std::cout << "\t\t" << "ld a, (de)"; break;
			case 0x2A: std::cout << "\t\t" << "ldi a, (hl)"; break;
			case 0x3A: std::cout << "\t\t" << "ldd a, (hl)"; break;

			case 0x0B: std::cout << "\t\t" << "dec bc"; break;
			case 0x1B: std::cout << "\t\t" << "dec de"; break;
			case 0x2B: std::cout << "\t\t" << "dec hl"; break;
			case 0x3B: std::cout << "\t\t" << "dec sp"; break;

			case 0x0C: std::cout << "\t\t" << "inc c"; break;
			case 0x1C: std::cout << "\t\t" << "inc e"; break;
			case 0x2C: std::cout << "\t\t" << "inc l"; break;
			case 0x3C: std::cout << "\t\t" << "inc a"; break;

			case 0x0D: std::cout << "\t\t" << "dec c"; break;
			case 0x1D: std::cout << "\t\t" << "dec e"; break;
			case 0x2D: std::cout << "\t\t" << "dec l"; break;
			case 0x3D: std::cout << "\t\t" << "dec a"; break;

			case 0x0E: std::cout << oh8(low) << "\t\t" << "ld c, $" << oh8(low); break;
			case 0x1E: std::cout << oh8(low) << "\t\t" << "ld e, $" << oh8(low); break;
			case 0x2E: std::cout << oh8(low) << "\t\t" << "ld l, $" << oh8(low); break;
			case 0x3E: std::cout << oh8(low) << "\t\t" << "ld a, $" << oh8(low); break;

			case 0x0F: std::cout << "\t\t" << "rrca"; break;
			case 0x1F: std::cout << "\t\t" << "rra"; break;
			case 0x2F: std::cout << "\t\t" << "cpl"; break;
			case 0x3F: std::cout << "\t\t" << "ccf"; break;



			case 0x40: std::cout << "\t\t" << "ld b, b"; break;
			case 0x41: std::cout << "\t\t" << "ld b, c"; break;
			case 0x42: std::cout << "\t\t" << "ld b, d"; break;
			case 0x43: std::cout << "\t\t" << "ld b, e"; break;
			case 0x44: std::cout << "\t\t" << "ld b, h"; break;
			case 0x45: std::cout << "\t\t" << "ld b, l"; break;
			case 0x46: std::cout << "\t\t" << "ld b, (hl)"; break;
			case 0x47: std::cout << "\t\t" << "ld b, a"; break;

			case 0x48: std::cout << "\t\t" << "ld c, b"; break;
			case 0x49: std::cout << "\t\t" << "ld c, c"; break;
			case 0x4A: std::cout << "\t\t" << "ld c, d"; break;
			case 0x4B: std::cout << "\t\t" << "ld c, e"; break;
			case 0x4C: std::cout << "\t\t" << "ld c, h"; break;
			case 0x4D: std::cout << "\t\t" << "ld c, l"; break;
			case 0x4E: std::cout << "\t\t" << "ld c, (hl)"; break;
			case 0x4F: std::cout << "\t\t" << "ld c, a"; break;

			case 0x50: std::cout << "\t\t" << "ld d, b"; break;
			case 0x51: std::cout << "\t\t" << "ld d, c"; break;
			case 0x52: std::cout << "\t\t" << "ld d, d"; break;
			case 0x53: std::cout << "\t\t" << "ld d, e"; break;
			case 0x54: std::cout << "\t\t" << "ld d, h"; break;
			case 0x55: std::cout << "\t\t" << "ld d, l"; break;
			case 0x56: std::cout << "\t\t" << "ld d, (hl)"; break;
			case 0x57: std::cout << "\t\t" << "ld d, a"; break;

			case 0x58: std::cout << "\t\t" << "ld e, b"; break;
			case 0x59: std::cout << "\t\t" << "ld e, c"; break;
			case 0x5A: std::cout << "\t\t" << "ld e, d"; break;
			case 0x5B: std::cout << "\t\t" << "ld e, e"; break;
			case 0x5C: std::cout << "\t\t" << "ld e, h"; break;
			case 0x5D: std::cout << "\t\t" << "ld e, l"; break;
			case 0x5E: std::cout << "\t\t" << "ld e, (hl)"; break;
			case 0x5F: std::cout << "\t\t" << "ld e, a"; break;

			case 0x60: std::cout << "\t\t" << "ld h, b"; break;
			case 0x61: std::cout << "\t\t" << "ld h, c"; break;
			case 0x62: std::cout << "\t\t" << "ld h, d"; break;
			case 0x63: std::cout << "\t\t" << "ld h, e"; break;
			case 0x64: std::cout << "\t\t" << "ld h, h"; break;
			case 0x65: std::cout << "\t\t" << "ld h, l"; break;
			case 0x66: std::cout << "\t\t" << "ld h, (hl)"; break;
			case 0x67: std::cout << "\t\t" << "ld h, a"; break;

			case 0x68: std::cout << "\t\t" << "ld l, b"; break;
			case 0x69: std::cout << "\t\t" << "ld l, c"; break;
			case 0x6A: std::cout << "\t\t" << "ld l, d"; break;
			case 0x6B: std::cout << "\t\t" << "ld l, e"; break;
			case 0x6C: std::cout << "\t\t" << "ld l, h"; break;
			case 0x6D: std::cout << "\t\t" << "ld l, l"; break;
			case 0x6E: std::cout << "\t\t" << "ld l, (hl)"; break;
			case 0x6F: std::cout << "\t\t" << "ld l, a"; break;

			case 0x70: std::cout << "\t\t" << "ld (hl), b"; break;
			case 0x71: std::cout << "\t\t" << "ld (hl), c"; break;
			case 0x72: std::cout << "\t\t" << "ld (hl), d"; break;
			case 0x73: std::cout << "\t\t" << "ld (hl), e"; break;
			case 0x74: std::cout << "\t\t" << "ld (hl), h"; break;
			case 0x75: std::cout << "\t\t" << "ld (hl), l"; break;
			case 0x76: std::cout << "\t\t" << "halt"; break;
			case 0x77: std::cout << "\t\t" << "ld (hl), a"; break;

			case 0x78: std::cout << "\t\t" << "ld a, b"; break;
			case 0x79: std::cout << "\t\t" << "ld a, c"; break;
			case 0x7A: std::cout << "\t\t" << "ld a, d"; break;
			case 0x7B: std::cout << "\t\t" << "ld a, e"; break;
			case 0x7C: std::cout << "\t\t" << "ld a, h"; break;
			case 0x7D: std::cout << "\t\t" << "ld a, l"; break;
			case 0x7E: std::cout << "\t\t" << "ld a, (hl)"; break;
			case 0x7F: std::cout << "\t\t" << "ld a, a"; break;



			case 0x80: std::cout << "\t\t" << "add a, b"; break;
			case 0x81: std::cout << "\t\t" << "add a, c"; break;
			case 0x82: std::cout << "\t\t" << "add a, d"; break;
			case 0x83: std::cout << "\t\t" << "add a, e"; break;
			case 0x84: std::cout << "\t\t" << "add a, h"; break;
			case 0x85: std::cout << "\t\t" << "add a, l"; break;
			case 0x86: std::cout << "\t\t" << "add a, (hl)"; break;
			case 0x87: std::cout << "\t\t" << "add a, a"; break;

			case 0x88: std::cout << "\t\t" << "adc a, b"; break;
			case 0x89: std::cout << "\t\t" << "adc a, c"; break;
			case 0x8A: std::cout << "\t\t" << "adc a, d"; break;
			case 0x8B: std::cout << "\t\t" << "adc a, e"; break;
			case 0x8C: std::cout << "\t\t" << "adc a, h"; break;
			case 0x8D: std::cout << "\t\t" << "adc a, l"; break;
			case 0x8E: std::cout << "\t\t" << "adc a, (hl)"; break;
			case 0x8F: std::cout << "\t\t" << "adc a, a"; break;

			case 0x90: std::cout << "\t\t" << "sub a, b"; break;
			case 0x91: std::cout << "\t\t" << "sub a, c"; break;
			case 0x92: std::cout << "\t\t" << "sub a, d"; break;
			case 0x93: std::cout << "\t\t" << "sub a, e"; break;
			case 0x94: std::cout << "\t\t" << "sub a, h"; break;
			case 0x95: std::cout << "\t\t" << "sub a, l"; break;
			case 0x96: std::cout << "\t\t" << "sub a, (hl)"; break;
			case 0x97: std::cout << "\t\t" << "sub a, a"; break;

			case 0x98: std::cout << "\t\t" << "sbc a, b"; break;
			case 0x99: std::cout << "\t\t" << "sbc a, c"; break;
			case 0x9A: std::cout << "\t\t" << "sbc a, d"; break;
			case 0x9B: std::cout << "\t\t" << "sbc a, e"; break;
			case 0x9C: std::cout << "\t\t" << "sbc a, h"; break;
			case 0x9D: std::cout << "\t\t" << "sbc a, l"; break;
			case 0x9E: std::cout << "\t\t" << "sbc a, (hl)"; break;
			case 0x9F: std::cout << "\t\t" << "sbc a, a"; break;

			case 0xA0: std::cout << "\t\t" << "and a, b"; break;
			case 0xA1: std::cout << "\t\t" << "and a, c"; break;
			case 0xA2: std::cout << "\t\t" << "and a, d"; break;
			case 0xA3: std::cout << "\t\t" << "and a, e"; break;
			case 0xA4: std::cout << "\t\t" << "and a, h"; break;
			case 0xA5: std::cout << "\t\t" << "and a, l"; break;
			case 0xA6: std::cout << "\t\t" << "and a, (hl)"; break;
			case 0xA7: std::cout << "\t\t" << "and a, a"; break;

			case 0xA8: std::cout << "\t\t" << "xor a, b"; break;
			case 0xA9: std::cout << "\t\t" << "xor a, c"; break;
			case 0xAA: std::cout << "\t\t" << "xor a, d"; break;
			case 0xAB: std::cout << "\t\t" << "xor a, e"; break;
			case 0xAC: std::cout << "\t\t" << "xor a, h"; break;
			case 0xAD: std::cout << "\t\t" << "xor a, l"; break;
			case 0xAE: std::cout << "\t\t" << "xor a, (hl)"; break;
			case 0xAF: std::cout << "\t\t" << "xor a, a"; break;

			case 0xB0: std::cout << "\t\t" << "or a, b"; break;
			case 0xB1: std::cout << "\t\t" << "or a, c"; break;
			case 0xB2: std::cout << "\t\t" << "or a, d"; break;
			case 0xB3: std::cout << "\t\t" << "or a, e"; break;
			case 0xB4: std::cout << "\t\t" << "or a, h"; break;
			case 0xB5: std::cout << "\t\t" << "or a, l"; break;
			case 0xB6: std::cout << "\t\t" << "or a, (hl)"; break;
			case 0xB7: std::cout << "\t\t" << "or a, a"; break;

			case 0xB8: std::cout << "\t\t" << "cp a, b"; break;
			case 0xB9: std::cout << "\t\t" << "cp a, c"; break;
			case 0xBA: std::cout << "\t\t" << "cp a, d"; break;
			case 0xBB: std::cout << "\t\t" << "cp a, e"; break;
			case 0xBC: std::cout << "\t\t" << "cp a, h"; break;
			case 0xBD: std::cout << "\t\t" << "cp a, l"; break;
			case 0xBE: std::cout << "\t\t" << "cp a, (hl)"; break;
			case 0xBF: std::cout << "\t\t" << "cp a, a"; break;



			case 0xC0: std::cout << "\t\t" << "ret nz"; break;
			case 0xD0: std::cout << "\t\t" << "ret nc"; break;
			case 0xE0: std::cout << oh8(low) << "\t\t" << "ldh ($" << oh8(low) << "), a"; break;
			case 0xF0: std::cout << oh8(low) << "\t\t" << "ldh a, ($" << oh8(low) << ")"; break;

			case 0xC1: std::cout << "\t\t" << "pop bc"; break;
			case 0xD1: std::cout << "\t\t" << "pop de"; break;
			case 0xE1: std::cout << "\t\t" << "pop hl"; break;
			case 0xF1: std::cout << "\t\t" << "pop af"; break;

			case 0xC2: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "jp nz, $" << oh16(value); break;
			case 0xD2: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "jp nc, $" << oh16(value); break;
			case 0xE2: std::cout << "\t\t" << "ldh (c), a"; break;
			case 0xF2: std::cout << "\t\t" << "ldh a, (c)"; break;

			case 0xC3: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "jp $" << oh16(value); break;
			case 0xD3: std::cout << "\t\t" << "INVALID OPCODE"; break;
			case 0xE3: std::cout << "\t\t" << "INVALID OPCODE"; break;
			case 0xF3: std::cout << "\t\t" << "di"; break;

			case 0xC4: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "call nz, $" << oh16(value); break;
			case 0xD4: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "call nc, $" << oh16(value); break;
			case 0xE4: std::cout << "\t\t" << "INVALID OPCODE"; break;
			case 0xF4: std::cout << "\t\t" << "INVALID OPCODE"; break;

			case 0xC5: std::cout << "\t\t" << "push bc"; break;
			case 0xD5: std::cout << "\t\t" << "push de"; break;
			case 0xE5: std::cout << "\t\t" << "push hl"; break;
			case 0xF5: std::cout << "\t\t" << "push af"; break;

			case 0xC6: std::cout << oh8(low) << "\t\t" << "add a, $" << oh8(low); break;
			case 0xD6: std::cout << oh8(low) << "\t\t" << "sub a, $" << oh8(low); break;
			case 0xE6: std::cout << oh8(low) << "\t\t" << "and a, $" << oh8(low); break;
			case 0xF6: std::cout << oh8(low) << "\t\t" << "or a, $" << oh8(low); break;

			case 0xC7: std::cout << "\t\t" << "rst $00"; break;
			case 0xD7: std::cout << "\t\t" << "rst $10"; break;
			case 0xE7: std::cout << "\t\t" << "rst $20"; break;
			case 0xF7: std::cout << "\t\t" << "rst $30"; break;

			case 0xC8: std::cout << "\t\t" << "ret z"; break;
			case 0xD8: std::cout << "\t\t" << "ret c"; break;
			case 0xE8: std::cout << oh8(low) << "\t\t" << "add sp, " << int(int8_t(low)); break;
			case 0xF8: std::cout << oh8(low) << "\t\t" << "ld hl, sp+" << int(int8_t(low)); break;

			case 0xC9: std::cout << "\t\t" << "ret"; break;
			case 0xD9: std::cout << "\t\t" << "reti"; break;
			case 0xE9: std::cout << "\t\t" << "jp hl"; break;
			case 0xF9: std::cout << "\t\t" << "ld sp, hl"; break;

			case 0xCA: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "jp z, $" << oh16(value); break;
			case 0xDA: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "jp c, $" << oh16(value); break;
			case 0xEA: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "ld ($" << oh16(value) << "), a"; break;
			case 0xFA: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "ld a, ($" << oh16(value) << ")"; break;

			case 0xCB: std::cout << oh8(low) << "\t\t";
				switch (low){
					case 0x00: std::cout << "rlc b"; break;
					case 0x01: std::cout << "rlc c"; break;
					case 0x02: std::cout << "rlc d"; break;
					case 0x03: std::cout << "rlc e"; break;
					case 0x04: std::cout << "rlc h"; break;
					case 0x05: std::cout << "rlc l"; break;
					case 0x06: std::cout << "rlc (hl)"; break;
					case 0x07: std::cout << "rlc a"; break;

					case 0x08: std::cout << "rrc b"; break;
					case 0x09: std::cout << "rrc c"; break;
					case 0x0A: std::cout << "rrc d"; break;
					case 0x0B: std::cout << "rrc e"; break;
					case 0x0C: std::cout << "rrc h"; break;
					case 0x0D: std::cout << "rrc l"; break;
					case 0x0E: std::cout << "rrc (hl)"; break;
					case 0x0F: std::cout << "rrc a"; break;

					case 0x10: std::cout << "rl b"; break;
					case 0x11: std::cout << "rl c"; break;
					case 0x12: std::cout << "rl d"; break;
					case 0x13: std::cout << "rl e"; break;
					case 0x14: std::cout << "rl h"; break;
					case 0x15: std::cout << "rl l"; break;
					case 0x16: std::cout << "rl (hl)"; break;
					case 0x17: std::cout << "rl a"; break;

					case 0x18: std::cout << "rr b"; break;
					case 0x19: std::cout << "rr c"; break;
					case 0x1A: std::cout << "rr d"; break;
					case 0x1B: std::cout << "rr e"; break;
					case 0x1C: std::cout << "rr h"; break;
					case 0x1D: std::cout << "rr l"; break;
					case 0x1E: std::cout << "rr (hl)"; break;
					case 0x1F: std::cout << "rr a"; break;

					case 0x20: std::cout << "sla b"; break;
					case 0x21: std::cout << "sla c"; break;
					case 0x22: std::cout << "sla d"; break;
					case 0x23: std::cout << "sla e"; break;
					case 0x24: std::cout << "sla h"; break;
					case 0x25: std::cout << "sla l"; break;
					case 0x26: std::cout << "sla (hl)"; break;
					case 0x27: std::cout << "sla a"; break;

					case 0x28: std::cout << "sra b"; break;
					case 0x29: std::cout << "sra c"; break;
					case 0x2A: std::cout << "sra d"; break;
					case 0x2B: std::cout << "sra e"; break;
					case 0x2C: std::cout << "sra h"; break;
					case 0x2D: std::cout << "sra l"; break;
					case 0x2E: std::cout << "sra (hl)"; break;
					case 0x2F: std::cout << "sra a"; break;

					case 0x30: std::cout << "swap b"; break;
					case 0x31: std::cout << "swap c"; break;
					case 0x32: std::cout << "swap d"; break;
					case 0x33: std::cout << "swap e"; break;
					case 0x34: std::cout << "swap h"; break;
					case 0x35: std::cout << "swap l"; break;
					case 0x36: std::cout << "swap (hl)"; break;
					case 0x37: std::cout << "swap a"; break;

					case 0x38: std::cout << "srl b"; break;
					case 0x39: std::cout << "srl c"; break;
					case 0x3A: std::cout << "srl d"; break;
					case 0x3B: std::cout << "srl e"; break;
					case 0x3C: std::cout << "srl h"; break;
					case 0x3D: std::cout << "srl l"; break;
					case 0x3E: std::cout << "srl (hl)"; break;
					case 0x3F: std::cout << "srl a"; break;

					case 0x40: std::cout << "bit 0, b"; break;
					case 0x41: std::cout << "bit 0, c"; break;
					case 0x42: std::cout << "bit 0, d"; break;
					case 0x43: std::cout << "bit 0, e"; break;
					case 0x44: std::cout << "bit 0, h"; break;
					case 0x45: std::cout << "bit 0, l"; break;
					case 0x46: std::cout << "bit 0, (hl)"; break;
					case 0x47: std::cout << "bit 0, a"; break;

					case 0x48: std::cout << "bit 1, b"; break;
					case 0x49: std::cout << "bit 1, c"; break;
					case 0x4A: std::cout << "bit 1, d"; break;
					case 0x4B: std::cout << "bit 1, e"; break;
					case 0x4C: std::cout << "bit 1, h"; break;
					case 0x4D: std::cout << "bit 1, l"; break;
					case 0x4E: std::cout << "bit 1, (hl)"; break;
					case 0x4F: std::cout << "bit 1, a"; break;

					case 0x50: std::cout << "bit 2, b"; break;
					case 0x51: std::cout << "bit 2, c"; break;
					case 0x52: std::cout << "bit 2, d"; break;
					case 0x53: std::cout << "bit 2, e"; break;
					case 0x54: std::cout << "bit 2, h"; break;
					case 0x55: std::cout << "bit 2, l"; break;
					case 0x56: std::cout << "bit 2, (hl)"; break;
					case 0x57: std::cout << "bit 2, a"; break;

					case 0x58: std::cout << "bit 3, b"; break;
					case 0x59: std::cout << "bit 3, c"; break;
					case 0x5A: std::cout << "bit 3, d"; break;
					case 0x5B: std::cout << "bit 3, e"; break;
					case 0x5C: std::cout << "bit 3, h"; break;
					case 0x5D: std::cout << "bit 3, l"; break;
					case 0x5E: std::cout << "bit 3, (hl)"; break;
					case 0x5F: std::cout << "bit 3, a"; break;

					case 0x60: std::cout << "bit 4, b"; break;
					case 0x61: std::cout << "bit 4, c"; break;
					case 0x62: std::cout << "bit 4, d"; break;
					case 0x63: std::cout << "bit 4, e"; break;
					case 0x64: std::cout << "bit 4, h"; break;
					case 0x65: std::cout << "bit 4, l"; break;
					case 0x66: std::cout << "bit 4, (hl)"; break;
					case 0x67: std::cout << "bit 4, a"; break;

					case 0x68: std::cout << "bit 5, b"; break;
					case 0x69: std::cout << "bit 5, c"; break;
					case 0x6A: std::cout << "bit 5, d"; break;
					case 0x6B: std::cout << "bit 5, e"; break;
					case 0x6C: std::cout << "bit 5, h"; break;
					case 0x6D: std::cout << "bit 5, l"; break;
					case 0x6E: std::cout << "bit 5, (hl)"; break;
					case 0x6F: std::cout << "bit 5, a"; break;

					case 0x70: std::cout << "bit 6, b"; break;
					case 0x71: std::cout << "bit 6, c"; break;
					case 0x72: std::cout << "bit 6, d"; break;
					case 0x73: std::cout << "bit 6, e"; break;
					case 0x74: std::cout << "bit 6, h"; break;
					case 0x75: std::cout << "bit 6, l"; break;
					case 0x76: std::cout << "bit 6, (hl)"; break;
					case 0x77: std::cout << "bit 6, a"; break;

					case 0x78: std::cout << "bit 7, b"; break;
					case 0x79: std::cout << "bit 7, c"; break;
					case 0x7A: std::cout << "bit 7, d"; break;
					case 0x7B: std::cout << "bit 7, e"; break;
					case 0x7C: std::cout << "bit 7, h"; break;
					case 0x7D: std::cout << "bit 7, l"; break;
					case 0x7E: std::cout << "bit 7, (hl)"; break;
					case 0x7F: std::cout << "bit 7, a"; break;

					case 0x80: std::cout << "res 0, b"; break;
					case 0x81: std::cout << "res 0, c"; break;
					case 0x82: std::cout << "res 0, d"; break;
					case 0x83: std::cout << "res 0, e"; break;
					case 0x84: std::cout << "res 0, h"; break;
					case 0x85: std::cout << "res 0, l"; break;
					case 0x86: std::cout << "res 0, (hl)"; break;
					case 0x87: std::cout << "res 0, a"; break;

					case 0x88: std::cout << "res 1, b"; break;
					case 0x89: std::cout << "res 1, c"; break;
					case 0x8A: std::cout << "res 1, d"; break;
					case 0x8B: std::cout << "res 1, e"; break;
					case 0x8C: std::cout << "res 1, h"; break;
					case 0x8D: std::cout << "res 1, l"; break;
					case 0x8E: std::cout << "res 1, (hl)"; break;
					case 0x8F: std::cout << "res 1, a"; break;

					case 0x90: std::cout << "res 2, b"; break;
					case 0x91: std::cout << "res 2, c"; break;
					case 0x92: std::cout << "res 2, d"; break;
					case 0x93: std::cout << "res 2, e"; break;
					case 0x94: std::cout << "res 2, h"; break;
					case 0x95: std::cout << "res 2, l"; break;
					case 0x96: std::cout << "res 2, (hl)"; break;
					case 0x97: std::cout << "res 2, a"; break;

					case 0x98: std::cout << "res 3, b"; break;
					case 0x99: std::cout << "res 3, c"; break;
					case 0x9A: std::cout << "res 3, d"; break;
					case 0x9B: std::cout << "res 3, e"; break;
					case 0x9C: std::cout << "res 3, h"; break;
					case 0x9D: std::cout << "res 3, l"; break;
					case 0x9E: std::cout << "res 3, (hl)"; break;
					case 0x9F: std::cout << "res 3, a"; break;

					case 0xA0: std::cout << "res 4, b"; break;
					case 0xA1: std::cout << "res 4, c"; break;
					case 0xA2: std::cout << "res 4, d"; break;
					case 0xA3: std::cout << "res 4, e"; break;
					case 0xA4: std::cout << "res 4, h"; break;
					case 0xA5: std::cout << "res 4, l"; break;
					case 0xA6: std::cout << "res 4, (hl)"; break;
					case 0xA7: std::cout << "res 4, a"; break;

					case 0xA8: std::cout << "res 5, b"; break;
					case 0xA9: std::cout << "res 5, c"; break;
					case 0xAA: std::cout << "res 5, d"; break;
					case 0xAB: std::cout << "res 5, e"; break;
					case 0xAC: std::cout << "res 5, h"; break;
					case 0xAD: std::cout << "res 5, l"; break;
					case 0xAE: std::cout << "res 5, (hl)"; break;
					case 0xAF: std::cout << "res 5, a"; break;

					case 0xB0: std::cout << "res 6, b"; break;
					case 0xB1: std::cout << "res 6, c"; break;
					case 0xB2: std::cout << "res 6, d"; break;
					case 0xB3: std::cout << "res 6, e"; break;
					case 0xB4: std::cout << "res 6, h"; break;
					case 0xB5: std::cout << "res 6, l"; break;
					case 0xB6: std::cout << "res 6, (hl)"; break;
					case 0xB7: std::cout << "res 6, a"; break;

					case 0xB8: std::cout << "res 7, b"; break;
					case 0xB9: std::cout << "res 7, c"; break;
					case 0xBA: std::cout << "res 7, d"; break;
					case 0xBB: std::cout << "res 7, e"; break;
					case 0xBC: std::cout << "res 7, h"; break;
					case 0xBD: std::cout << "res 7, l"; break;
					case 0xBE: std::cout << "res 7, (hl)"; break;
					case 0xBF: std::cout << "res 7, a"; break;

					case 0xC0: std::cout << "set 0, b"; break;
					case 0xC1: std::cout << "set 0, c"; break;
					case 0xC2: std::cout << "set 0, d"; break;
					case 0xC3: std::cout << "set 0, e"; break;
					case 0xC4: std::cout << "set 0, h"; break;
					case 0xC5: std::cout << "set 0, l"; break;
					case 0xC6: std::cout << "set 0, (hl)"; break;
					case 0xC7: std::cout << "set 0, a"; break;

					case 0xC8: std::cout << "set 1, b"; break;
					case 0xC9: std::cout << "set 1, c"; break;
					case 0xCA: std::cout << "set 1, d"; break;
					case 0xCB: std::cout << "set 1, e"; break;
					case 0xCC: std::cout << "set 1, h"; break;
					case 0xCD: std::cout << "set 1, l"; break;
					case 0xCE: std::cout << "set 1, (hl)"; break;
					case 0xCF: std::cout << "set 1, a"; break;

					case 0xD0: std::cout << "set 2, b"; break;
					case 0xD1: std::cout << "set 2, c"; break;
					case 0xD2: std::cout << "set 2, d"; break;
					case 0xD3: std::cout << "set 2, e"; break;
					case 0xD4: std::cout << "set 2, h"; break;
					case 0xD5: std::cout << "set 2, l"; break;
					case 0xD6: std::cout << "set 2, (hl)"; break;
					case 0xD7: std::cout << "set 2, a"; break;

					case 0xD8: std::cout << "set 3, b"; break;
					case 0xD9: std::cout << "set 3, c"; break;
					case 0xDA: std::cout << "set 3, d"; break;
					case 0xDB: std::cout << "set 3, e"; break;
					case 0xDC: std::cout << "set 3, h"; break;
					case 0xDD: std::cout << "set 3, l"; break;
					case 0xDE: std::cout << "set 3, (hl)"; break;
					case 0xDF: std::cout << "set 3, a"; break;

					case 0xE0: std::cout << "set 4, b"; break;
					case 0xE1: std::cout << "set 4, c"; break;
					case 0xE2: std::cout << "set 4, d"; break;
					case 0xE3: std::cout << "set 4, e"; break;
					case 0xE4: std::cout << "set 4, h"; break;
					case 0xE5: std::cout << "set 4, l"; break;
					case 0xE6: std::cout << "set 4, (hl)"; break;
					case 0xE7: std::cout << "set 4, a"; break;

					case 0xE8: std::cout << "set 5, b"; break;
					case 0xE9: std::cout << "set 5, c"; break;
					case 0xEA: std::cout << "set 5, d"; break;
					case 0xEB: std::cout << "set 5, e"; break;
					case 0xEC: std::cout << "set 5, h"; break;
					case 0xED: std::cout << "set 5, l"; break;
					case 0xEE: std::cout << "set 5, (hl)"; break;
					case 0xEF: std::cout << "set 5, a"; break;

					case 0xF0: std::cout << "set 6, b"; break;
					case 0xF1: std::cout << "set 6, c"; break;
					case 0xF2: std::cout << "set 6, d"; break;
					case 0xF3: std::cout << "set 6, e"; break;
					case 0xF4: std::cout << "set 6, h"; break;
					case 0xF5: std::cout << "set 6, l"; break;
					case 0xF6: std::cout << "set 6, (hl)"; break;
					case 0xF7: std::cout << "set 6, a"; break;

					case 0xF8: std::cout << "set 7, b"; break;
					case 0xF9: std::cout << "set 7, c"; break;
					case 0xFA: std::cout << "set 7, d"; break;
					case 0xFB: std::cout << "set 7, e"; break;
					case 0xFC: std::cout << "set 7, h"; break;
					case 0xFD: std::cout << "set 7, l"; break;
					case 0xFE: std::cout << "set 7, (hl)"; break;
					case 0xFF: std::cout << "set 7, a"; break;
				}
				break;
			case 0xDB: std::cout << "\t\t" << "INVALID OPCODE"; break;
			case 0xEB: std::cout << "\t\t" << "INVALID OPCODE"; break;
			case 0xFB: std::cout << "\t\t" << "ei"; break;

			case 0xCC: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "call z, $" << oh16(value); break;
			case 0xDC: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "call c, $" << oh16(value); break;
			case 0xEC: std::cout << "\t\t" << "INVALID OPCODE"; break;
			case 0xFC: std::cout << "\t\t" << "INVALID OPCODE"; break;

			case 0xCD: std::cout << oh8(low) << " " << oh8(high) << "\t\t" << "call $" << oh16(value); break;
			case 0xDD: std::cout << "\t\t" << "INVALID OPCODE"; break;
			case 0xED: std::cout << "\t\t" << "INVALID OPCODE"; break;
			case 0xFD: std::cout << "\t\t" << "INVALID OPCODE"; break;

			case 0xCE: std::cout << oh8(low) << "\t\t" << "adc a, $" << oh8(low); break;
			case 0xDE: std::cout << oh8(low) << "\t\t" << "sbc a, $" << oh8(low); break;
			case 0xEE: std::cout << oh8(low) << "\t\t" << "xor a, $" << oh8(low); break;
			case 0xFE: std::cout << oh8(low) << "\t\t" << "cp a, $" << oh8(low); break;

			case 0xCF: std::cout << "\t\t" << "rst $08"; break;
			case 0xDF: std::cout << "\t\t" << "rst $18"; break;
			case 0xEF: std::cout << "\t\t" << "rst $28"; break;
			case 0xFF: std::cout << "\t\t" << "rst $38"; break;
		}

		std::cout << "\t\taf: " << oh16(reg_af) << ", bc: " << oh16(reg_bc) << ", de: " << oh16(reg_de) << ", hl: " << oh16(reg_hl) << ", sp: " << oh16(reg_sp);

		std::cout << "\t\tstack: ";
		for (uint16_t pointer = m_sp; pointer < 0xFFF4; pointer += 2){
			uint16_t low = m_memory->get(pointer);
			uint16_t high = m_memory->get(pointer + 1);
			uint16_t value = (high << 8) | low;
			std::cout << oh16(value) << " ";
		}
		std::cout << std::endl;
	}
}
