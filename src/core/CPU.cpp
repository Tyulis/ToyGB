#include "core/CPU.hpp"

// Opcode identifiers for the main 8-bits registers
#define reg_id_b 0
#define reg_id_c 1
#define reg_id_d 2
#define reg_id_e 3
#define reg_id_h 4
#define reg_id_l 5
#define reg_id_f 6
#define reg_id_a 7

// Shortcuts for the main 8-bits registers. Can be directly assigned to.
#define reg_b m_registers[reg_id_b]
#define reg_c m_registers[reg_id_c]
#define reg_d m_registers[reg_id_d]
#define reg_e m_registers[reg_id_e]
#define reg_h m_registers[reg_id_h]
#define reg_l m_registers[reg_id_l]
#define reg_f m_registers[reg_id_f]
#define reg_a m_registers[reg_id_a]

// Opcode identifiers for the coupled 16-bits registers
#define reg_id_bc 0
#define reg_id_de 1
#define reg_id_hl 2
#define reg_id_sp 3

// Shortcuts for the coupled 16-bits registers values. Can NOT be directly assigned to
#define reg_bc ((reg_b << 8) | reg_c)
#define reg_de ((reg_d << 8) | reg_e)
#define reg_hl ((reg_h << 8) | reg_l)
#define reg_af ((reg_a << 8) | reg_f)
#define reg_sp m_sp

// Shortcuts for the status flags values. Can NOT be directly assigned to.
#define flag_z ((reg_f >> 7) & 1)
#define flag_n ((reg_f >> 6) & 1)
#define flag_h ((reg_f >> 5) & 1)
#define flag_c ((reg_f >> 4) & 1)

// Bitmasks to get each flag from the f register
#define mask_flag_z 0b10000000
#define mask_flag_n 0b01000000
#define mask_flag_h 0b00100000
#define mask_flag_c 0b00010000
#define mask_flags (mask_flag_z | mask_flag_n | mask_flag_h | mask_flag_c)

// Special value to tell setFlags(...) to leave a flag unchanged
#define UNAFFECTED 0xFF


// DETAILS ABOUT THE CARRY (c) AND HALF-CARRY FLAG (h) CALCULATION :
// For 8-bits operations, the half-carry flag is whether a carry have been carried from the lower to the upper half of the byte
// Here, it is calculated in a rather simple way : compare the new value of the lower nibble with its former value
// For increasing operations (inc, add, adc : flag n = 0), it is h = (new & 0x0F) < (old & 0x0F), because if there was a carry from the lower to the upper nibble
// the lower nibble always end up with a value lower than before (because it's new = old + operand - 0x10, with old and operand < 0x10 so operand - 0x10 < 0),
// and without half-carry, as it is an increasing operation the value is always higher (or equal if the operand's lower nibble is 0) than before, so the logic holds
// Conversely, for decreasing operations (dec, sub, sbc, cp), the logic is reversed : h = (new & 0x0F) > (old & 0x0F)
// With no half-carry, the new value must be lower or equal than the old one
// With a half-carry, the new value is new = old - operand + 0x10 with old and operand < 0x10, so -operand + 0x10 > 0, thus the new value is always higher than the previous one.

// Calculate the new H flag (half-carry) value from the previous and new value of the accumulator. _INC for increasing operations (inc, add, adc), _DEC for decreasing (dec, sub, sbc, cp)
#define HALF_CARRY_INC(oldvalue, newvalue) ((newvalue & 0x0F) < (oldvalue & 0x0F))
#define HALF_CARRY_DEC(oldvalue, newvalue) ((newvalue & 0x0F) > (oldvalue & 0x0F))


namespace toygb {
	// Initialize the component with null values, the actual initialization is in CPU::init
	CPU::CPU() {
		m_hram = nullptr;
		m_wram = nullptr;

		m_hdmaMapping = nullptr;
		m_wramMapping = nullptr;
		m_hramMapping = nullptr;
		m_wramBankMapping = nullptr;
		m_systemControlMapping = nullptr;

		m_cyclesToSkip = 0;
	}

	CPU::CPU(GameboyConfig& config) {
		m_config = config;

		m_hram = nullptr;
		m_wram = nullptr;

		m_hdmaMapping = nullptr;
		m_wramMapping = nullptr;
		m_wramBankMapping = nullptr;
		m_systemControlMapping = nullptr;
		m_hramMapping = nullptr;

		m_cyclesToSkip = 0;
	}

	CPU::~CPU() {
		if (m_hram != nullptr) delete[] m_hram;
		if (m_wram != nullptr) delete[] m_wram;
		if (m_bootrom.size > 0) delete[] m_bootrom.bootrom;
		m_hram = m_wram = nullptr;

		if (m_hramMapping != nullptr) delete m_hramMapping;
		if (m_wramMapping != nullptr) delete m_wramMapping;
		if (m_systemControlMapping != nullptr) delete m_systemControlMapping;
		if (m_wramBankMapping != nullptr) delete m_wramBankMapping;
		if (m_hdmaMapping != nullptr) delete m_hdmaMapping;
		m_hramMapping = nullptr;
		m_wramMapping = nullptr;
		m_wramBankMapping = nullptr;
		m_systemControlMapping = nullptr;
		m_hdmaMapping = nullptr;
	}

	// Initialize the component
	void CPU::init(HardwareStatus* hardware, InterruptVector* interrupt) {
		m_hardware = hardware;
		m_interrupt = interrupt;

		// Load the bootrom if set
		initRegisters();
		bool hasBootrom = loadBootrom(m_config.bootrom);
		m_hardware->setBootrom(hasBootrom);  // CPU must be initialized before everything else such that the bootrom mode is transferred to others too

		m_hram = new uint8_t[HRAM_SIZE];
		switch (hardware->mode()) {
			case OperationMode::DMG:  // DMG mode : only one WRAM bank
				m_wram = new uint8_t[WRAM_SIZE]; break;
			case OperationMode::CGB:  // CGB mode : 8 switchable WRAM banks
				m_wram = new uint8_t[WRAM_BANK_SIZE * WRAM_BANK_NUM]; break;
			case OperationMode::Auto:
				throw EmulationError("OperationMode::Auto given to CPU");
		}

		m_bootromDisableMapping = new BootromDisableMapping(m_hardware);
		m_hramMapping = new ArrayMemoryMapping(m_hram);
		switch (hardware->mode()) {
			case OperationMode::DMG:
				m_wramMapping = new ArrayMemoryMapping(m_wram);
				break;
			case OperationMode::CGB:
				m_wramBank = 1;  // According to the bootROM
				m_systemControlMapping = new SystemControlMapping(m_hardware);
				m_wramBankMapping = new WRAMBankSelectMapping(&m_wramBank);
				m_wramMapping = new FixBankedMemoryMapping(&m_wramBank, WRAM_BANK_NUM, WRAM_BANK_SIZE, m_wram, true);
				m_hdmaMapping = new HDMAMapping();
				break;
			case OperationMode::Auto:
				throw EmulationError("OperationMode::Auto given to CPU");
		}
	}

	// Configure the memory mappings associated with the component
	void CPU::configureMemory(MemoryMap* memory) {
		memory->add(HRAM_OFFSET, HRAM_OFFSET + HRAM_SIZE - 1, m_hramMapping);

		memory->add(IO_BOOTROM_UNMAP, IO_BOOTROM_UNMAP, m_bootromDisableMapping);
		if (m_hardware->mode() == OperationMode::CGB) {
			memory->add(IO_WRAM_BANK, IO_WRAM_BANK, m_wramBankMapping);
			memory->add(IO_HDMA_SOURCEHIGH, IO_HDMA_SETTINGS, m_hdmaMapping);
			memory->add(IO_KEY0, IO_KEY1, m_systemControlMapping);
		}
		memory->add(WRAM_OFFSET, WRAM_OFFSET + WRAM_SIZE - 1, m_wramMapping);
		memory->add(ECHO_OFFSET, ECHO_OFFSET + ECHO_SIZE - 1, m_wramMapping);  // Same mapping at a different address range to emulate WRAM echo
	}

// Skip `num` clocks
#define cycle(num) m_cyclesToSkip = num-1; \
				co_await std::suspend_always();


	// Main CPU loop, as a coroutine
	GBComponent CPU::run(MemoryMap* memory, DMAController* dma) {
		m_memory = memory;
		m_dma = dma;

		m_ei_scheduled = false;
		m_haltBug = false;
		m_halted = false;
		m_haltCycles = 0;
		m_lastStatMode = 0;

		// Start CPU operation
		uint8_t opcode = memoryRead(m_pc++);  // Start with first opcode already fetched

		try {
			while (true) {
				// Halt the program and transfer data when type = 0 (general-purpose) or type = 1 (HBlank) and STAT mode is 0 (HBlank) (and was not 0 before, so if we just entered HBlank)
				if (m_hardware->isCGBCapable() && m_hdmaMapping->running() && (!m_hdmaMapping->type || ((m_memory->get(IO_LCD_STATUS) & 0b11) == 0 && m_lastStatMode != 0))) {
					cycle(1);  // 4 clocks of overhead
					// Transfer one 16-bytes block per HBlank in HBlank DMA mode, all at once for GDMA
					uint8_t blocksToTransfer = (m_hdmaMapping->type == 1 ? 1 : m_hdmaMapping->blocks);
					for (int block = 0; block < blocksToTransfer; block++) {
						uint16_t source = m_hdmaMapping->source;
						uint16_t dest = m_hdmaMapping->dest;

						// Write one byte every 2 clocks. As our CPU clock goes 4 by 4 clocks, we write 2 bytes every 4 clocks but it might be little bit inaccurate
						for (uint16_t i = 0x0000; i < 0x0010; i += 2) {
							// HDMA reads invalid values when its source address is within VRAM
							// From The Cycle-Accurate Gameboy Doc, it writes 2 garbage bytes on CGB and AGB, 1 garbage byte on AGS, then all 0xFF
							// FIXME : Write corrupted bytes at every block or only once at the beginning of the transfer ?
							if ((source & 0xE000) == 0x8000) {
								if (i == 0x0000) {
									// FIXME : What kind of garbage ? Currently, some arbitrary values
									m_memory->set(dest + i, 0x01);
									if (m_hardware->console() == ConsoleModel::AGS)
										m_memory->set(dest + i + 1, 0xFF);
									else
										m_memory->set(dest + i + 1, 0x02);
								} else {
									m_memory->set(dest + i, 0xFF);
									m_memory->set(dest + i + 1, 0xFF);
								}
							} else {
								m_memory->set(dest + i, m_memory->get(source + i));
								m_memory->set(dest + i + 1, m_memory->get(source + i + 1));
							}
							cycle(1);
						}

						m_hdmaMapping->nextBlock();
					}

					m_lastStatMode = m_memory->get(IO_LCD_STATUS) & 3;
					continue;
				}

				if (m_hardware->isCGBCapable())
					m_lastStatMode = m_memory->get(IO_LCD_STATUS) & 3;

				// Interrupt management
				Interrupt interrupt = m_interrupt->getInterrupt();
				if (interrupt != Interrupt::None) {  // There is a requested and active interrupt (IE + IF, IME is still not checked)
					if (m_halted)  // Get out of halt mode, even if IME is not set
						m_halted = false;

					// Jump to interrupt vector only when IME is set
					if (m_interrupt->getMaster()) {
						cycle(2);
						m_interrupt->resetRequest(interrupt);
						m_interrupt->setMaster(false);  // Interrupts are disabled before jumping to the interrupt vector

						// Push PC onto the stack before jumping
						m_sp -= 1;
						memoryWrite(m_sp--, (m_pc - 1) >> 8); cycle(1);
						memoryWrite(m_sp, (m_pc - 1) & 0xFF); cycle(1);

						// Standard interrupt vectors
						switch (interrupt) {
							case Interrupt::VBlank:  m_pc = 0x0040; break;
							case Interrupt::LCDStat: m_pc = 0x0048; break;
							case Interrupt::Timer:   m_pc = 0x0050; break;
							case Interrupt::Serial:  m_pc = 0x0058; break;
							case Interrupt::Joypad:  m_pc = 0x0060; break;
							case Interrupt::None: break;
						}
						cycle(1);
						opcode = memoryRead(m_pc++); cycle(1);  // Fetch the next opcode
						continue;
					}
				}

				// EI has a 1-CPU cycle delay before actually activating the interrupts
				if (m_ei_scheduled) {
					m_ei_scheduled = false;
					m_interrupt->setMaster(true);
				}

				uint16_t basePC = m_pc - 1;

				// Continue the program
				if (!m_halted) {
					if (m_config.disassemble && m_hardware->bootromUnmapped())
						logDisassembly(basePC);

					// Opcode description :
					// Binary opcode | hex opcodes | mnemonic | description | CPU cycles (*4 for clocks) | flag changes (znhc, 0 is reset, 1 is set, - is unaffected, z/n/h/c = it depends, x = depends on the actual instruction)
					// Here, a cycle is always taken to fetch the next opcode at the end of the loop code, so a single-cycle instruction code will not contain any cycle(), and a multi-cycle instruction will have one less than necessary
					// It would have been nice to split this in individual functions, but as we need to tick the clock (= suspend the coroutine) at specific times, we need to do all this in the body of the coroutine
					// Here timings are more-or-less what they should (that is, a cycle for each memory access, there might be sub-CPU cycle timing issues but as of now it seems to be okay), other sub-instruction technicalities that happen entirely within the CPU shouldn't be a problem anyway
					// FIXME : Tick the cycle before or after the memory access ? Currently, after.
					// The Gameboy CPU is little-endian : in memory, 16-bits values are stored lower byte first (| --- | low | high | --- |)

					// Extract the first 2 bits, that split the available opcodes in 4 distinct blocks, to optimize our opcode decoding a little bit
					uint8_t opblock = opcode >> 6;

					////////// Opcodes in 0b00xxxxxx : Mostly control, 16-bits operations, inc, dec and utilities
					if (opblock == 0b00) {
						// 00 000000 | 0x00 | nop | Do nothing for a cycle | 1 | ----
						if (opcode == 0b00000000) {
							// nop
						}

						// 00 01 0000 | 0x10 | stop | Stop the clock to get into a very low-power mode (or to switch to CGB double-speed mode) | 2 | ----
						else if (opcode == 0b00010000) {
							// If a button is pressed and selected (so if at least one bit is 0)
							if ((m_memory->get(IO_JOYPAD) & 0x0F) < 0x0F) {
								// No interrupt pending : 2-bytes opcode, enter halt mode, no divider reset
								if (interrupt == Interrupt::None) {
									m_pc += 1; cycle(1);
									m_halted = true;
								}
								// Else : nothing (1-byte opcode, no mode change, no divider reset)
							}
							// No button pressed and selected
							else {
								// Speed switch requested
								if (m_systemControlMapping->prepareSpeedSwitch()) {
									// No interrupt pending : trigger speed switch, then enter halt mode
									if (interrupt == Interrupt::None) {
										m_pc += 1; cycle(1);
										m_hardware->resetDivider();
										m_hardware->triggerSpeedSwitch();
										m_halted = true;
										// Halt mode for 32768 CPU cycles, but the first 2050 are in complete stop
										m_haltCycles = 32768 - 2050;
									}

									// Interrupt pending during the speed switch
									else {
										// IME enabled : the CPU glitches non-deterministically ?
										if (m_interrupt->getMaster()) {
											continue;  // Just hang
										}
										// IME disabled : 1-byte opcode, no mode change, speed switch, divider reset
										else {
											m_hardware->resetDivider();
											m_hardware->triggerSpeedSwitch();
										}
									}
								}
								// No pending speed switch : enter stop mode, reset the divider
								else {
									m_hardware->setStopMode(true);
									m_hardware->resetDivider();
									// No interrupt pending : 2-bytes opcode
									if (interrupt == Interrupt::None) {
										m_pc += 1; cycle(1);
									}
									// Interrupt pending : 1-byte opcode
								}
							}
						}

						// 001 cc 000 | 0x20, 0x28, 0x30, 0x38 | jr [nz, z, nc, c], s8 | Conditional relative jump, by a number of bytes given by the given signed value | 3 (jump) / 2 (condition is false, not jump) | ----
						else if ((opcode & 0b11100111) == 0b00100000) {
							uint8_t condition = (opcode >> 3) & 3;
							int8_t diff = int8_t(memoryRead(m_pc++)); cycle(1);  // Get the signed displacement
							if (checkCondition(condition)) {
								m_pc += diff;  // The displacement is from the value of PC AFTER fetching both the JR opcode and its operand
								cycle(1);
							}
						}

						// 00 rr 0001 | 0x01, 0x11, 0x21, 0x31 | ld rr, u16 | Load an immediate 16-bits value into a 16-bits register | 3 | ----
						else if ((opcode & 0b11001111) == 0b00000001) {
							uint8_t low = memoryRead(m_pc++); cycle(1);
							uint8_t high = memoryRead(m_pc++); cycle(1);
							uint8_t identifier = (opcode >> 4) & 0b11;
							set16(identifier, high, low);
						}

						// 00 10 0010 | 0x22 | ldi (hl), a / ld (hl+), a | Load the value of A into the memory address given by HL, then increment HL by 1 | 2 | ----
						else if (opcode == 0b00100010) {
							memoryWrite(reg_hl, reg_a); cycle(1);
							increment16(&reg_h, &reg_l);
						}

						// 00 11 0010 | 0x32 | ldd (hl, a) / ld (hl-), a | Load the value of A into the memory address given by HL, then decrement HL by 1 | 2 | ----
						else if (opcode == 0b00110010) {
							memoryWrite(reg_hl, reg_a); cycle(1);
							decrement16(&reg_h, &reg_l);
						}

						// 00 rr 0010 | 0x02, 0x12 | ld (rr), a | Load the value of A into the memory address given by BC or DE | 2 | ----
						else if ((opcode & 0b11101111) == 0b00000010) {  // 00 00 0010 = ld (bc), a
							uint8_t identifier = (opcode >> 4) & 0b11;  // BC and DE use standard identifiers, those that should have been HL and SP are ldi and ldd and are handled separately
							uint16_t address = get16(identifier);
							memoryWrite(address, reg_a); cycle(1);
						}

						// Those are handled directly instead of using the identifier and make it generic, to handle OAM corruption properly (TODO)
						// 00 00 0011 | 0x03 | inc bc | Increment the 16-bits value of BC by 1 | 2 | ----
						else if (opcode == 0b00000011) {
							cycle(1);
							increment16(&reg_b, &reg_c);
						}

						// 00 01 0011 | 0x13 | inc de | Increment the 16-bits value of DE by 1 | 2 | ----
						else if (opcode == 0b00010011) {
							cycle(1);
							increment16(&reg_d, &reg_e);
						}

						// 00 10 0011 | 0x23 | inc hl | Increment the 16-bits value of HL by 1 | 2 | ----
						else if (opcode == 0b00100011) {  // 00 10 0011 = inc hl
							cycle(1);
							increment16(&reg_h, &reg_l);
						}

						// 00 11 0011 | 0x33 | inc sp | Increment the value of SP by 1 | 2 | ----
						else if (opcode == 0b00110011) {  // 00 11 0011 = inc sp
							cycle(1);
							m_sp += 1;
						}

						// 00 110 100 | 0x34 | inc (hl) | Increment the value at the memory address given by HL by 1 | 3 | z0h-
						else if (opcode == 0b00110100) {
							uint8_t value = memoryRead(reg_hl); cycle(1);
							uint8_t result = value + 1;
							memoryWrite(reg_hl, result); cycle(1);
							setFlags(result == 0, 0, HALF_CARRY_INC(value, result), UNAFFECTED);
						}

						// 00 rrr 100 | 0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x3C | inc r | Increment the value of a register by 1 | 1 | z0h-
						else if ((opcode & 0b11000111) == 0b00000100) {
							uint8_t reg = (opcode >> 3) & 7;
							uint8_t value = m_registers[reg];
							uint8_t result = value + 1;
							m_registers[reg] = result;
							setFlags(result == 0, 0, HALF_CARRY_INC(value, result), UNAFFECTED);
						}

						// 00 110 101 | 0x35 | dec (hl) | Decrement the value at the memory address given by HL by 1 | 3 | z1h-
						else if (opcode == 0b00110101) {
							uint8_t value = memoryRead(reg_hl); cycle(1);
							uint8_t result = value - 1;
							memoryWrite(reg_hl, result); cycle(1);
							setFlags(result == 0, 1, HALF_CARRY_DEC(value, result), UNAFFECTED);
						}

						// 00 rrr 101 | 0x05, 0x0D, 0x15, 0x1D, 0x25, 0x2D, 0x3D | dec r | Decrement the value of a register by 1 | 1 | z1h-
						else if ((opcode & 0b11000111) == 0b00000101) {
							uint8_t reg = (opcode >> 3) & 7;
							uint8_t value = m_registers[reg];
							uint8_t result = value - 1;
							m_registers[reg] = result;
							setFlags(result == 0, 1, HALF_CARRY_DEC(value, result), UNAFFECTED);
						}

						// 00 110 110 | 0x36 | ld (hl), u8 | Load an immediate value into the memory address given by HL | 3 | ----
						else if (opcode == 0b00110110) {
							uint8_t value = memoryRead(m_pc++); cycle(1);
							memoryWrite(reg_hl, value); cycle(1);
						}

						// 00 rrr 110 | 0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x3E | ld r, u8 | Load an immediate value into a register | 2 | ----
						else if ((opcode & 0b11000111) == 0b00000110) {
							uint8_t value = memoryRead(m_pc++); cycle(1);
							uint8_t destreg = (opcode >> 3) & 7;
							m_registers[destreg] = value;
						}

						// 00 00 0111 | 0x07 | rlca | Rotate the accumulator's bits left (c 76543210 -> 7 65432107) | 1 | 000c
						else if (opcode == 0b00000111) {
							reg_a = (reg_a << 1) | (reg_a >> 7);
							setFlags(0, 0, 0, reg_a & 1);
						}

						// 00 01 0111 | 0x17 | rla | Rotate the accumulator and carry bits left (c 76543210 -> 7 6543210c) | 1 | 000c
						else if (opcode == 0b00010111) {
							bool newcarry = reg_a >> 7;
							reg_a = (reg_a << 1) | flag_c;
							setFlags(0, 0, 0, newcarry);
						}

						// 00 10 0111 | 0x27 | daa | For Binary-Coded Decimal value (e.g 0x75 for the decimal value 75), adjust the value back to BCD after an arithmetical operation with another BCD operands | 1 | z-0c
						//                         | Example (decimal : 75 + 19 = 94) : 0x75 + 0x19 = 0x8E -- daa -> 0x94
						else if (opcode == 0b00100111) {
							applyDAA();
						}

						// 00 11 0111 | 0x37 | scf | Set the carry flag | 1 | -001
						else if (opcode == 0b00110111) {
							reg_f |= mask_flag_c;  // set carry
							reg_f &= ~(mask_flag_n | mask_flag_h);  // clear n and h flags
							//setFlags(UNAFFECTED, 0, 0, 1);
						}

						// 00 00 1000 | 0x08 | ld (u16), sp | Load the value of SP into a 16-bits immediate address | 5 | ----
						else if (opcode == 0b00001000) {
							uint16_t low = memoryRead(m_pc++); cycle(1);
							uint16_t high = memoryRead(m_pc++); cycle(1);
							uint16_t address = (high << 8) | low;
							memoryWrite(address, m_sp & 0xFF); cycle(1);
							memoryWrite(address + 1, m_sp >> 8); cycle(1);
						}

						// 00 01 1000 | 0x18 | jr s8 | Unconditional relative jump, by a number of bytes given by an immediate signed displacement | 3 | ----
						else if (opcode == 0b00011000) {  // 00 01 1000 = jr e
							int8_t diff = int8_t(memoryRead(m_pc++)); cycle(1);
							m_pc += diff;  // The displacement is from the value of PC AFTER fetching both the JR opcode and its operand
							cycle(1);
						}

						// 00 rr 1001 | 0x09, 0x19, 0x29, 0x39 | add hl, rr | Add the value of a 16-register to HL | 2 | -0hc
						else if ((opcode & 0b11001111) == 0b00001001) {
							uint8_t identifier = (opcode >> 4) & 0b11;
							uint16_t result = reg_hl + get16(identifier); cycle(1);
							// Internally, it is a shorthand for add l, c ; adc h, b ; so flags are set for the upper bytes
							setFlags(UNAFFECTED, 0, (result & 0x0FFF) < (reg_hl & 0x0FFF), result < reg_hl);
							reg_h = result >> 8;
							reg_l = result & 0xFF;
						}

						// 00 10 1010 | 0x2A | ldi a, (hl) / ld a, (hl+) | Load the value at the address given by HL into register A, then increment HL by 1 | 2 | ----
						else if (opcode == 0b00101010) {
							uint8_t value = memoryRead(reg_hl); cycle(1);
							increment16(&reg_h, &reg_l);
							reg_a = value;
						}

						// 00 11 1010 | 0x3A | ldd a, (hl) / ld a, (hl-) | Load the value at the address given by HL into register A, then decrement HL by 1 | 2 | ----
						else if (opcode == 0b00111010){  // 00 11 1010 = ldd a, (hl)
							uint8_t value = memoryRead(reg_hl); cycle(1);
							decrement16(&reg_h, &reg_l);
							reg_a = value;
						}

						// 00 rr 1010 | 0x0A, 0x1A | ld a, (rr) | Load the value at the address given by the value of a 16-bits register into A | 2 | ----
						else if ((opcode & 0b11001111) == 0b00001010) {
							uint8_t identifier = (opcode >> 4) & 0b11;  // BC and DE use standard identifiers, those that should have been HL and SP are ldi and ldd and are handled separately
							uint8_t value = memoryRead(get16(identifier)); cycle(1);
							reg_a = value;
						}

						// Those are handled directly instead of using the identifier and make it generic, to handle OAM corruption properly (TODO)
						// 00 00 1011 | 0x0B | dec bc | Decrement the value of BC by 1 | 2 | ----
						else if (opcode == 0b00001011) {
							cycle(1);
							decrement16(&reg_b, &reg_c);
						}

						// 00 01 1011 | 0x1B | dec de | Decrement the value of DE by 1 | 2 | ----
						else if (opcode == 0b00011011) {
							cycle(1);
							decrement16(&reg_d, &reg_e);
						}

						// 00 10 1011 | 0x2B | dec hl | Decrement the value of HL by 1 | 2 | ----
						else if (opcode == 0b00101011) {
							cycle(1);
							decrement16(&reg_h, &reg_l);
						}

						// 00 11 1011 | 0x3B | dec sp | Decrement the value of SP by 1 | 2 | ----
						else if (opcode == 0b00111011) {
							cycle(1);
							m_sp -= 1;
						}

						// 00 00 1111 | 0x0F | rrca | Rotate the accumulator's bits right (76543210 c -> 07654321 0) | 1 | 000c
						else if (opcode == 0b00001111) {
							reg_a = (reg_a >> 1) | (reg_a << 7);
							setFlags(0, 0, 0, reg_a >> 7);
						}

						// 00 01 1111 | 0x1F | rra | Rotate the accumulator's and carry bits right (76543210 c -> c7654321 0) | 1 | 000c
						else if (opcode == 0b00011111) {
							bool newcarry = reg_a & 1;
							reg_a = (reg_a >> 1) | (flag_c << 7);
							setFlags(0, 0, 0, newcarry);
						}

						// 00 10 1111 | 0x2F | cpl | Take the complement of the accumulator (flip all bits) | 1 | -11-
						else if (opcode == 0b00101111) {
							reg_a = ~reg_a;  // flip A
							setFlags(UNAFFECTED, 1, 1, UNAFFECTED);
						}

						// 00 11 1111 | 0x3F | ccf | Take the complement of the carry flag (flip flag c) | 1 | -00c
						else if (opcode == 0b00111111) {
							reg_f ^= mask_flag_c;  // flip carry
							setFlags(UNAFFECTED, 0, 0, UNAFFECTED);
						}
					}

					////////// Opcodes in 0b01xxxxxx : Load instructions
					else if (opblock == 0b01) {
						// 01 110 110 | 0x76 | halt | Put the CPU in halt mode (low-power mode where it does nothing) until an interrupt is requested and enabled (IE + IF, not necessarily IME) | 1 | ----
						if (opcode == 0b01110110) {
							if (m_interrupt->getMaster() || m_interrupt->getInterrupt() == Interrupt::None) {
								m_halted = true;
							} else {
								// If the Interrupt Master Enable (ei/di) is clear and there is a pending interrupt (IE + IF), a hardware glitch makes it not enter halt mode and not increment PC after fetching the next instruction
								m_haltBug = true;
							}
						}

						// 01 rrr 110 | 0x46, 0x4E, 0x56, 0x5E, 0x66, 0x6E, 0x7E | ld r, (hl) | Load the value at the memory address given by HL into a register | 2 | ----
						else if ((opcode & 0b11000111) == 0b01000110) {
							uint8_t value = memoryRead(reg_hl); cycle(1);
							uint8_t destreg = (opcode >> 3) & 7;
							m_registers[destreg] = value;
						}

						// 01 110 rrr | 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x77 | ld (hl), r | Load the value of a register into memory at the address given by HL | 2 | ----
						else if ((opcode & 0b11111000) == 0b01110000){
							uint8_t sourcereg = opcode & 7;
							memoryWrite(reg_hl, m_registers[sourcereg]); cycle(1);
						}

						// 01 xxx yyy | All other values in 0x40-0x7F | ld x, y | Load the value of register y into register x | 1 | ----
						else if ((opcode & 0b11000000) == 0b01000000){
							uint8_t sourcereg = opcode & 7;
							uint8_t destreg = (opcode >> 3) & 7;
							m_registers[destreg] = m_registers[sourcereg];
						}
					}

					////////// Opcodes in 0b10xxxxxx : Arithmetical instructions : Details in CPU::accumulatorOperation
					else if (opblock == 0b10) {
						// 10 ppp 110 | 0x86, 0x8E, 0x96, 0x9E, 0xA6, 0xAE, 0xB6, 0xBE | <op> a, (hl) | Do an arithmetical operation between the accumulator and the value in memory at the address given by HL and put the result back in the accumulator | 2 | xxxx
						if ((opcode & 0b11000111) == 0b10000110) {
							uint8_t operation = (opcode >> 3) & 7;
							uint8_t operand = memoryRead(reg_hl); cycle(1);
							accumulatorOperation(operation, operand);
						}

						// 10 ppp rrr | All other values in 0x80-0xBF | <op> a, r | Do an arithmetical operation between the accumulator and another register and put the result back in the accumulator | 1 | xxxx
						else if ((opcode & 0b11000000) == 0b10000000) {
							uint8_t operation = (opcode >> 3) & 7;
							uint8_t reg = opcode & 7;
							accumulatorOperation(operation, m_registers[reg]);
						}
					}

					////////// Opcodes in 0b11xxxxxx : Mostly control, stack and immediate value instructions
					else if (opblock == 0b11) {
						// 110 cc 000 | 0xC0, 0xC8, 0xD0, 0xD8 | ret [nz, z, nc, c] | Conditional return, pop the value of PC from the stack if the condition is true | 5 (return, condition is true) / 2 (false) | ----
						if ((opcode & 0b11100111) == 0b11000000) {
							uint8_t condition = (opcode >> 3) & 3; cycle(1);
							if (checkCondition(condition)) {
								uint16_t low = memoryRead(m_sp++); cycle(1);
								uint16_t high = memoryRead(m_sp++); cycle(1);
								uint16_t address = (high << 8) | low; cycle(1);
								m_pc = address;
							}
						}

						// 11 10 0000 | 0xE0 | ldh (u8), a | Load the value of register A into memory at address 0xFF00 + u8 | 3 | ----
						else if (opcode == 0b11100000) {
							uint16_t low = memoryRead(m_pc++); cycle(1);
							uint16_t address = 0xFF00 | low;
							memoryWrite(address, reg_a); cycle(1);
						}

						// 11 11 0000 | 0xF0 | ldh a, (u8) | Load the value at memory address 0xFF00 + u8 into register A | 3 | ----
						else if (opcode == 0b11110000) {
							uint16_t low = memoryRead(m_pc++); cycle(1);
							uint16_t address = 0xFF00 | low;
							reg_a = memoryRead(address); cycle(1);
						}

						// 11 11 0001 | 0xF1 | pop af | Pop a 16-bits value from the stack into 16-bits register AF (that replaces SP as identifier 0b11 here) | 3 | znhc
						else if (opcode == 0b11110001) {
							// The lower 4 bits of F are not only unused, they physically don't exist, so we need to mask them out
							reg_f = memoryRead(m_sp++) & 0xF0; cycle(1);
							reg_a = memoryRead(m_sp++); cycle(1);
						}

						// 11 rr 0001 | 0xC1, 0xD1, 0xE1 | pop rr | Pop a 16-bits value from the stack into a 16-bits register | 3 | ----
						else if ((opcode & 0b11001111) == 0b11000001) {
							uint8_t low = memoryRead(m_sp++); cycle(1);
							uint8_t high = memoryRead(m_sp++); cycle(1);
							uint8_t identifier = (opcode >> 4) & 0b11;
							set16(identifier, high, low);
						}

						// 110 cc 010 | 0xC2, 0xCA, 0xD2, 0xDA | jp [nz, z, nc, c], u16 | Conditional absolute jump, jump to an immediate 16-bits address if the condition is true | 4 (jump, condition is true) / 3 (false) | ----
						else if ((opcode & 0b11100111) == 0b11000010) {
							uint8_t condition = (opcode >> 3) & 3;
							uint16_t low = memoryRead(m_pc++); cycle(1);
							uint16_t high = memoryRead(m_pc++); cycle(1);
							uint16_t address = (high << 8) | low;
							if (checkCondition(condition)) {
								cycle(1);
								m_pc = address;
							}
						}

						// 11 10 0010 | 0xE2 | ldh (c), a | Load the value of A into the address (0xFF00 + value of the register C) | 2 | ----
						else if (opcode == 0b11100010) {
							uint16_t address = 0xFF00 | reg_c;
							memoryWrite(address, reg_a); cycle(1);
						}

						// 11 11 0010 | 0xF2 | ldh a, (c) | Load the value at address (0xFF00 + value of the register C) into the register A | 2 | ----
						else if (opcode == 0b11110010) {
							uint16_t address = 0xFF00 | reg_c;
							reg_a = memoryRead(address); cycle(1);
						}

						// 11 00 0011 | 0xC3 | jp u16 | Unconditional absolute jump to an immediate 16-bits address | 4 | ----
						else if (opcode == 0b11000011) {
							uint16_t low = memoryRead(m_pc++); cycle(1);
							uint16_t high = memoryRead(m_pc++); cycle(1);
							uint16_t address = (high << 8) | low;
							cycle(1);
							m_pc = address;
						}

						// 11 11 0011 | 0xF3 | di | Immediately clear IME (Interrupts Master Enable) : Until ei or reti are executed, requested and enabled interrupts stay pending and to not trigger a jump to the interrupt vector | 1 | ----
						else if (opcode == 0b11110011) {
							m_ei_scheduled = false;  // Cancel a potential ei instruction executed at the previous cycle
							m_interrupt->setMaster(false);
						}

						// 110 cc 100 | 0xC4, 0xCC, 0xD4, 0xDC | call [nz, z, nc, c], u16 | Conditonally call a subroutine at an immediate 16-bits address. If the condition is true, PC is pushed on the stack then jumps | 6 (call, condition is true) / 3 (false) | ----
						else if ((opcode & 0b11100111) == 0b11000100) {
							uint8_t condition = (opcode >> 3) & 3;
							// The jump address is always loaded, regardless of the condition
							uint16_t low = memoryRead(m_pc++); cycle(1);
							uint16_t high = memoryRead(m_pc++); cycle(1);
							if (checkCondition(condition)) {
								uint16_t address = (high << 8) | low;
								m_sp -= 1; cycle(1);
								memoryWrite(m_sp--, m_pc >> 8); cycle(1);
								memoryWrite(m_sp, m_pc & 0xFF); cycle(1);
								m_pc = address;
							}
						}

						// 11 11 0101 | 0xF5 | push af | Push the value of the 16-bits register AF onto the stack | 4 | ----
						else if (opcode == 0b11110101) {
							m_sp -= 1;
							cycle(1);
							memoryWrite(m_sp--, reg_a); cycle(1);
							memoryWrite(m_sp, reg_f); cycle(1);
						}

						// 11 rr 0101 | 0xC5, 0xD5, 0xE5 | push rr | Push the value of a 16-bits register onto the stack | 4 | ----
						else if ((opcode & 0b11001111) == 0b11000101) {
							m_sp -= 1;
							cycle(1);
							uint8_t identifier = (opcode >> 4) & 0b11;
							uint16_t value = get16(identifier);
							memoryWrite(m_sp--, value >> 8); cycle(1);
							memoryWrite(m_sp, value & 0xFF); cycle(1);
						}

						// 11 ppp 110 | 0xC6, 0xCE, 0xD6, 0xDE, 0xE6, 0xEE, 0xF6, 0xFE | <op> a, u8 | Perform an arithmetical operation between the accumulator and an immediate value, and put the result back into the accumulator | 2 | xxxx
						else if ((opcode & 0b11000111) == 0b11000110) {
							uint8_t operation = (opcode >> 3) & 7;
							uint8_t operand = memoryRead(m_pc++); cycle(1);
							accumulatorOperation(operation, operand);
						}

						// 11 xxx 111 | 0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF | rst xx | Call a reset vector (0x0000 / 0x0008 / 0x0010 / 0x0018 / 0x0020 / 0x0028 / 0x0030 / 0x0038) | 4 | ----
						else if ((opcode & 0b11000111) == 0b11000111) {
							uint16_t address = opcode & 0b00111000;  // Reset routine address happens to be exactly those 3 bits shifted left by 3 bits
							m_sp -= 1; cycle(1);
							// Push PC onto the stack before jumping
							memoryWrite(m_sp--, m_pc >> 8); cycle(1);
							memoryWrite(m_sp, m_pc & 0xFF); cycle(1);
							m_pc = address;
						}

						// 11 10 1000 | 0xE8 | add sp, s8 | Add a signed 8-bits immediate value to the value of SP | 4 | 00hc
						else if (opcode == 0b11101000) {
							uint16_t operand = uint16_t(int16_t(int8_t(memoryRead(m_pc++)))); cycle(1);  // All this just converts the 8-bits two-complements operand into its 16-bits two-complements equivalent
							uint16_t result = m_sp + operand; cycle(1);
							// Flags H and C are calculated for the lower byte
							setFlags(0, 0, (m_sp & 0x000F) + (operand & 0x000F) > 0x000F, (m_sp & 0x00FF) + (operand & 0x00FF) > 0x00FF);
							m_sp = result; cycle(1);
						}

						// 11 11 1000 | 0xF8 | ld hl, sp+s8 | Load the value of (SP + signed 8-bits immediate value) into HL | 3 | 00hc
						else if (opcode == 0b11111000) {
							uint16_t operand = uint16_t(int16_t(int8_t(memoryRead(m_pc++)))); cycle(1);  // All this just converts the 8-bits two-complements operand into its 16-bits two-complements equivalent
							uint16_t result = m_sp + operand; cycle(1);
							// Flags H and C are calculated for the lower byte
							setFlags(0, 0, (m_sp & 0x000F) + (operand & 0x000F) > 0x000F, (m_sp & 0x00FF) + (operand & 0x00FF) > 0x00FF);
							reg_h = result >> 8;
							reg_l = result & 0xFF;
						}

						// 11 00 1001 | 0xC9 | ret | Unconditionally return from a subroutine | 4 | ----
						else if (opcode == 0b11001001) {
							// Pop PC from the stack and jump to it
							uint16_t low = memoryRead(m_sp++); cycle(1);
							uint16_t high = memoryRead(m_sp++); cycle(1);
							uint16_t address = (high << 8) | low;
							m_pc = address; cycle(1);
						}

						// 11 01 1001 | 0xD9 | reti | Unconditionally return from a subroutine and enable interrupts (set IME) | 4 | ----
						else if (opcode == 0b11011001) {
							// Pop PC from the stack and jump to it
							uint16_t low = memoryRead(m_sp++); cycle(1);
							uint16_t high = memoryRead(m_sp++); cycle(1);
							uint16_t address = (high << 8) | low;  // Contrary to ei, there is no additional delay for reti (there is probably one but hidden in the 4 cycles reti takes)
							m_pc = address; cycle(1);
							m_interrupt->setMaster(true);
						}

						// 11 10 1001 | 0xE9 | jp hl | Unconditonal jump to the address given by HL | 1 | ----
						else if (opcode == 0b11101001) {
							m_pc = reg_hl;
						}

						// 11 11 1001 | 0xF9 | ld sp, hl | Load the value of HL into SP | 2 | ----
						else if (opcode == 0b11111001) {
							cycle(1);
							m_sp = reg_hl;
						}

						// 11 10 1010 | 0xEA | ld (u16), a | Load the value of a into an immediate memory address | 4 | ----
						else if (opcode == 0b11101010) {
							uint16_t low = memoryRead(m_pc++); cycle(1);
							uint16_t high = memoryRead(m_pc++); cycle(1);
							uint16_t address = (high << 8) | low;
							memoryWrite(address, reg_a); cycle(1);
						}

						// 11 11 1010 | 0xFA | ld a, (u16) | Load the value at an immediate memory address into register A | 4 | ----
						else if (opcode == 0b11111010) {
							uint16_t low = memoryRead(m_pc++); cycle(1);
							uint16_t high = memoryRead(m_pc++); cycle(1);
							uint16_t address = (high << 8) | low;
							uint8_t value = memoryRead(address); cycle(1);
							reg_a = value;
						}

						// 11 00 1011 | 0xCB | Prefix for bitwise operations, specific opcode is the next byte | 2 / 4 (with (hl)) | xxxx
						else if (opcode == 0b11001011) {
							// The specific opcode is 0bBBPPPRRR, with BB a block of instructions (like the normal ones), PPP the parameter within that block, and RRR the 8-bits register (or (hl) for 010) to operate onto
							uint8_t operation = memoryRead(m_pc++); cycle(1);
							uint8_t reg = operation & 7;

							uint8_t operand;
							if (reg == 0b110) {
								operand = memoryRead(reg_hl); cycle(1);
							} else {
								operand = m_registers[reg];
							}

							uint8_t result = operand;
							uint8_t block = (operation >> 6) & 3;
							uint8_t subop = (operation >> 3) & 7;

							// Block 0b00xxxRRR : Bitwise shifts and rotations
							if (block == 0b00) {
								// 00 000 rrr | 0x00-0x07 | rlc r | Rotate the bits of a register left (c 76543210 -> 7 65432107)| 2/4 | z00c
								if (subop == 0b000) {
									result = (operand << 1) | (operand >> 7);
									setFlags(result == 0, 0, 0, operand >> 7);  // The new carry is the bit that was shifted out
								}

								// 00 001 rrr | 0x08-0x0F | rrc r | Rotate the bits of a register right (76543210 c -> 07654321 0) | 2/4 | z00c
								else if (subop == 0b001) {
									result = (operand >> 1) | (operand << 7);
									setFlags(result == 0, 0, 0, operand & 1);  // The new carry is the bit that got shifted out
								}

								// 00 010 rrr | 0x10-0x17 | rl r | Rotate the bits of a register and carry left (c 76543210 -> 7 6543210c) | 2/4 | z00c
								else if (subop == 0b010) {
									result = (operand << 1) | flag_c;
									setFlags(result == 0, 0, 0, operand >> 7);
								}

								// 00 011 rrr | 0x18-0x1F | rr r | Rotate the bits of a register and carry right (76543210 c -> c7654321 0) | 2/4 | z00c
								else if (subop == 0b011) {
									result = (operand >> 1) | (flag_c << 7);
									setFlags(result == 0, 0, 0, operand & 1);
								}

								// 00 100 rrr | 0x20-0x27 | sla r | Shift the bits of a register left (c mnopqrst -> m nopqrst0) | 2/4 | z00c
								else if (subop == 0b100) {
									result = operand << 1;
									setFlags(result == 0, 0, 0, operand >> 7);
								}

								// 00 101 rrr | 0x28-0x2F | sra r | Shift the bits of a register right, leaving the leftmost bit at its initial value (mnopqrst c -> mmnopqrs t) | 2/4 | z00c
								else if (subop == 0b101) {
									result = (operand >> 1) | (operand & 0b10000000);
									setFlags(result == 0, 0, 0, operand & 1);
								}

								// 00 110 rrr | 0x30-0x37 | swap r | Swap the upper and lower nibbles of a register (76543210 -> 32107654) | 2/4 | z000
								else if (subop == 0b110) {
									result = ((operand & 0x0F) << 4) | ((operand & 0xF0) >> 4);
									setFlags(result == 0, 0, 0, 0);
								}

								// 00 111 rrr | 0x38-0x3F | srl l | Shift the bits of a register right, leaving zero in the leftmost bit (mnopqrst c -> 0mnopqrs t) | 2/4 | z00c
								else if (subop == 0b111){  // 00 111 : srl
									result = operand >> 1;
									setFlags(result == 0, 0, 0, operand & 1);
								}
							}

							// 01 bbb rrr | All values in 0x40-0x7F | bit b, r | Check the value of bit b of the value of a register. Bit = 0 -> flag z = 1, bit = 1 -> flag z = 0 | 2/4 | z01-
							else if (block == 0b01) {
								setFlags(((operand >> subop) & 1) == 0, 0, 1, UNAFFECTED);
							}

							// 10 bbb rrr | All values in 0x80-0xBF | res b, r | Reset (set to 0) bit b of the value of a register | 2/4 | ----
							else if (block == 0b10) {
								result = operand & ~(1 << subop);  // Mask out the given bit (like bit 2 -> 0b00000100 -> value is AND-ed by 0b11111011)
							}

							// 11 bbb rrr | All values in 0xC0-0xFF | set b, r | Set (to 1) bit b of the value of a register | 2/4 | ----
							else if (block == 0b11) {
								result = operand | (1 << subop);  // Mask in the given bit
							}


							// Write the result back to the original register (except for bit that only checks without changing the value)
							if (block != 0b01) {
								if (reg == 0b110) {  // 010 -> (hl)
									memoryWrite(reg_hl, result); cycle(1);
								} else {
									m_registers[reg] = result;
								}
							}
						}

						// 11 11 1011 | 0xFB | ei | Enable interrupts (set the Interrupt Master Enable), with a delay of 1 cycle | 1 | ----
						else if (opcode == 0b11111011) {
							m_ei_scheduled = true;
						}

						// 11 00 1101 | 0xCD | call u16 | Unconditionally call a subroutine at an immediate address | 6 | ----
						else if (opcode == 0b11001101) {
							uint16_t low = memoryRead(m_pc++); cycle(1);
							uint16_t high = memoryRead(m_pc++); cycle(1);
							uint16_t address = (high << 8) | low;
							m_sp -= 1; cycle(1);
							memoryWrite(m_sp--, m_pc >> 8); cycle(1);
							memoryWrite(m_sp, m_pc & 0xFF); cycle(1);
							m_pc = address;
						}

						// Undefined opcodes 0xD3, 0xE3, 0xE4, 0xF4, 0xDB, 0xEB, 0xEC, 0xFC, 0xDD, 0xED, 0xFD hang the CPU (TODO : make a proper debug of this and just hang the CPU)
						else {
							std::stringstream errstream;
							errstream << "Undefined opcode " << oh8(opcode);
							throw EmulationError(errstream.str());
						}
					}

					if (m_config.disassemble && m_hardware->bootromUnmapped())
						logStatus();

					opcode = memoryRead(m_pc); cycle(1);  // Fetch the next opcode during the last cycle of the current instruction
					if (!m_haltBug)  // When a halt instruction is executed when an interrupt is pending (IF + IE) and IME is clear, halt mode is not entered and PC is not incremented after the next fetch
						m_pc += 1;
					else
						m_haltBug = false;
				} else {  // Skip the cycle if in halt or stop mode
					cycle(1);
					if (m_haltCycles > 0) {
						m_haltCycles -= 1;
						if (m_haltCycles == 0)
							m_halted = false;
					}
				}
			}
		} catch (const _EmulationError& exc){
			std::cout << std::endl << exc.what() << std::endl;
			throw exc;
		}
	}

	// Load the bootrom into memory and tell whether it was successful
	bool CPU::loadBootrom(std::string filename) {
		if (filename.empty())
			m_bootrom = getBootrom(m_config, m_hardware);
		else
			m_bootrom = getBootrom(filename);

		// Could not load the bootrom,
		if (m_bootrom.size < 0) {
			m_hardware->setBootromStatus(true);
			return false;
		} else {
			m_hardware->setBootromStatus(false);
			m_pc = 0x0000;
			return true;
		}
	}

	// Initialize the CPU status with default values for the hardware, if there is no bootrom
	void CPU::initRegisters() {
		reg_sp = 0xFFFE;
		m_pc = 0x0100;
		switch (m_hardware->console()){
			case ConsoleModel::DMG:
				if (m_hardware->system() == SystemRevision::DMG_0){
					reg_a = 0x01; reg_f = 0x00;
					reg_b = 0xFF; reg_c = 0x13;
					reg_d = 0x00; reg_e = 0xC1;
					reg_h = 0x84; reg_l = 0x03;
				} else {
					reg_a = 0x01; reg_f = 0x80;  // f is 10??0000
					reg_b = 0x00; reg_c = 0x13;
					reg_d = 0x00; reg_e = 0xD8;
					reg_h = 0x01; reg_l = 0x4D;
				}
				break;

			case ConsoleModel::MGB:
				reg_a = 0xFF; reg_f = 0x80;  // f is 10??0000
				reg_b = 0x00; reg_c = 0x13;
				reg_d = 0x00; reg_e = 0xD8;
				reg_h = 0x01; reg_l = 0x4D;
				break;

			case ConsoleModel::SGB:
				reg_a = 0x01; reg_f = 0x00;
				reg_b = 0x00; reg_c = 0x14;
				reg_d = 0x00; reg_e = 0x00;
				reg_h = 0xC0; reg_l = 0x60;
				break;

			case ConsoleModel::SGB2:
				reg_a = 0xFF; reg_f = 0x00;
				reg_b = 0x00; reg_c = 0x14;
				reg_d = 0x00; reg_e = 0x00;
				reg_h = 0xC0; reg_l = 0x60;
				break;

			case ConsoleModel::CGB:
				if (m_hardware->mode() == OperationMode::DMG){
					reg_a = 0x11; reg_f = 0x80;
					reg_b = 0x00; reg_c = 0x00;  // b depends on the cartridge
					reg_d = 0x00; reg_e = 0x08;
					reg_h = 0x00; reg_l = 0x7C;  // May be 0x991A in some cases
				} else {
					reg_a = 0x11; reg_f = 0x80;
					reg_b = 0x00; reg_c = 0x00;
					reg_d = 0xFF; reg_e = 0x56;
					reg_h = 0x00; reg_l = 0x0D;
				}
				break;

			case ConsoleModel::AGB:
			case ConsoleModel::AGS:
			case ConsoleModel::GBP:
				if (m_hardware->mode() == OperationMode::DMG){
					reg_a = 0x11; reg_f = 0x00;  // f depends on the cartridge
					reg_b = 0x01; reg_c = 0x00;  // b depends on the cartridge
					reg_d = 0x00; reg_e = 0x08;
					reg_h = 0x00; reg_l = 0x7C;  // May be 0x991A in some cases
				} else {
					reg_a = 0x11; reg_f = 0x00;
					reg_b = 0x01; reg_c = 0x00;
					reg_d = 0xFF; reg_e = 0x56;
					reg_h = 0x00; reg_l = 0x0D;
				}
				break;
			case ConsoleModel::Auto:
				throw EmulationError("ConsoleModel::Auto given to CPU");
		}
	}

	// Read the value at the given absolute memory address
	uint8_t CPU::memoryRead(uint16_t address) {
		// Overlaying the bootrom with our memory mapping system would be a pain
		// As only the CPU uses the ROM area (except the DMA component but AFAIK there's no DMA in the bootroms), it makes no difference to just hack it right there
		// CGB bootroms are larger than 0x0100, so they are mapped over 0x0000-0x00FF, leave the cartridge on 0x0100-0x01FF and mapped after 0x0200
		if (!m_hardware->bootromUnmapped() && (address < 0x0100 || (address >= 0x0200 && address < m_bootrom.size)))
			return m_bootrom.bootrom[address];

		// Handle bus conflicts during OAM DMA
		// For reference : https://www.reddit.com/r/EmuDev/comments/5hahss/gb_readwrite_memory_during_an_oam_dma/
		if (m_dma->isOAMDMAActive()) {
			if (address >= OAM_OFFSET && address < IO_OFFSET)
				return 0xFF;
			else
				return m_dma->conflictingRead(address);
		}

		return m_memory->get(address);
	}

	// Write a value at the given absolute memory address
	// FIXME : While the bootrom is mapped, are writes in its area fully ignored or are they still sent to the MBC ? (probably not)
	void CPU::memoryWrite(uint16_t address, uint8_t value) {
		// FIXME : What happens if a bus conflict happens between the CPU and DMA ?
		if (m_dma->isOAMDMAActive()) {
			if ((address >= OAM_OFFSET && address < IO_OFFSET) || m_dma->isConflicting(address))
				return;  // Probably ignored, but don’t actually know
		}
		m_memory->set(address, value);
	}

	// Check a jump condition, and return the result
	bool CPU::checkCondition(uint8_t condition) {
		switch (condition) {
			case 0b00:  // nz : flag z must be clear
				return !flag_z;
			case 0b01:  // z : flag z must be set
				return flag_z;
			case 0b10:  // nc : flag c must be clear
				return !flag_c;
			case 0b11:  // c : flag c must be set
				return flag_c;
		}

		std::stringstream errstream;
		errstream << "Invalid jump condition " << int(condition);
		throw EmulationError(errstream.str());
	}

	// Set flags to the given values. Arguments may be 0, 1, or UNAFFECTED to leave them unchanged
	void CPU::setFlags(uint8_t z, uint8_t n, uint8_t h, uint8_t c) {
		// Compute this simply with two masks. Example : setFlags(1, 0, UNAFFECTED, UNAFFECTED) -> reg_f = (0b01010000 | 0b1000000) & 0b10110000 = 0b11010000 & 0b10110000 = 0b10010000
		uint8_t setmask = ((z == 1) << 7) | ((n == 1) << 6) | ((h == 1) << 5) | ((c == 1) << 4);
		uint8_t resetmask = ~(((z == 0) << 7) | ((n == 0) << 6) | ((h == 0) << 5) | ((c == 0) << 4) | 0b00001111);
		reg_f = (reg_f | setmask) & resetmask;
	}

	// Perform an arithmetical operation between the accumulator and the given operand
	void CPU::accumulatorOperation(uint8_t operation, uint8_t operand) {
		uint8_t result;
		switch (operation) {
			// -- 000 xxx | add a, x | Add the values of the accumulator and the operand | z0hc
			case 0b000:
				result = reg_a + operand;
				setFlags(result == 0, 0, HALF_CARRY_INC(reg_a, result), (result < reg_a));
				reg_a = result;
				break;

			// -- 001 xxx | adc a, x | Add the values of the accumulator, the operand and the carry flag | z0hc
			case 0b001: {
				result = reg_a + operand + flag_c;
				// The usual comparison logic does not work here because of the potention +1 due to the carry, so we do it the old way, checking manually if there's a carry
				bool newhalf = (reg_a & 0x0f) + (operand & 0x0f) + flag_c > 0x0f;
				bool newcarry = uint16_t(reg_a) + uint16_t(operand) + uint16_t(flag_c) > 0x00FF;
				setFlags(result == 0, 0, newhalf, newcarry);
				reg_a = result;
				break;
			}
			// -- 010 xxx | sub a, x | Substract the values of the accumulator and the operand | z1hc
			case 0b010:  // sub
				result = reg_a - operand;
				setFlags(result == 0, 1, HALF_CARRY_DEC(reg_a, result), (result > reg_a));
				reg_a = result;
				break;

			// -- 011 xxx | sbc a, x | Substract the values of the accumulator, the operand and the carry flag | z1hc
			case 0b011: {
				result = reg_a - operand - flag_c;
				// The usual comparison logic does not work here because of the potention -1 due to the carry, so we do it the old way, checking manually if there's a carry
				bool newhalf = int8_t(reg_a & 0xf) - int8_t(operand & 0xf) - int8_t(flag_c) < 0;
				bool newcarry = int16_t(reg_a) - int16_t(operand) - int16_t(flag_c) < 0;
				setFlags(result == 0, 1, newhalf, newcarry);
				reg_a = result;
				break;
			}
			// -- 100 xxx | and a, x | Perform a bitwise AND between the accumulator and the operand | z010
			case 0b100:
				reg_a &= operand;
				setFlags(reg_a == 0, 0, 1, 0);
				break;

			// -- 101 xxx | xor a, x | Perform a bitwise XOR between the accumulator and the operand | z000
			case 0b101:
				reg_a ^= operand;
				setFlags(reg_a == 0, 0, 0, 0);
				break;

			// -- 110 xxx | or a, x | Perform a bitwise OR between the accumulator and the operand | z000
			case 0b110:
				reg_a |= operand;
				setFlags(reg_a == 0, 0, 0, 0);
				break;
			// -- 111 xxx | cp a, x | Compare the accumulator with the operand. Basically execute a sub instruction without putting the result back in the accumulator | z1hc
			case 0b111:
				result = reg_a - operand;
				setFlags(result == 0, 1, HALF_CARRY_DEC(reg_a, result), (result > reg_a));
				break;
			default:
				std::stringstream errstream;
				errstream << "Invalid 8-bit arithmetical operation " << int(operation);
				throw EmulationError(errstream.str());
				break;
		}
	}

	// TODO : OAM glitch
	// Increment the value of a 16-bits register by 1
	void CPU::increment16(uint8_t* high, uint8_t* low) {
		uint16_t value = (*high << 8) | *low;
		value += 1;
		*high = value >> 8;
		*low = value & 0xFF;
	}

	// TODO : OAM glitch
	// Decrement the value of a 16-bits register by 1
	void CPU::decrement16(uint8_t* high, uint8_t* low) {
		uint16_t value = (*high << 8) | *low;
		value -= 1;
		*high = value >> 8;
		*low = value & 0xFF;
	}

	// Get the value of a 16-bits register, as specified by the 2-bits identifier found in the opcode
	uint16_t CPU::get16(uint8_t identifier) {
		switch (identifier) {
			case reg_id_bc: return reg_bc;
			case reg_id_de: return reg_de;
			case reg_id_hl: return reg_hl;
			case reg_id_sp: return reg_sp;
			default:
				throw EmulationError("Invalid 16-bits register identifier given to CPU::get16");
		}
	}

	// Set the value of a 16-bits register, as specified by the 2-bits identifier found in the opcode
	void CPU::set16(uint8_t identifier, uint16_t value) {
		if (identifier == reg_id_sp){
			reg_sp = value;
		} else {
			uint8_t high = (value >> 8);
			uint8_t low = (value & 0xFF);
			set16(identifier, high, low);
		}
	}

	// Set the value of a 16-bits register, as specified by the 2-bits identifier found in the opcode
	void CPU::set16(uint8_t identifier, uint8_t high, uint8_t low) {
		switch (identifier) {
			case reg_id_bc:
				reg_b = high; reg_c = low;
				break;
			case reg_id_de:
				reg_d = high; reg_e = low;
				break;
			case reg_id_hl:
				reg_h = high; reg_l = low;
				break;
			case reg_id_sp:
				reg_sp = (high << 8) | low;
				break;
		}
	}

	// Execute the DAA instruction, that adjusts the result of an arithmetical operation between two Binary-Coded Decimal values back to decimal
	// The table is as follows, here the logic has been fully reduced, contrary to the tables that are usually in documentations (that may be what the hardware is doing internally, but we are not the hardware)
	// `-` Means regardless of that value (input), or unchanged (output)
	// N | C | H |     A | Lower nibble of A | Value to add to the accumulator | New value of the carry flag
	// 0 | 1 | - |     - |                 - | 0x60                            | 1
	// 0 | - | - | >0x99 |                 - | 0x60                            | 1
	// 0 | - | 1 |     - |                 - | 0x06                            | -
	// 0 | - | - |     - |                >9 | 0x06                            | -
	// 1 | 1 | 1 |     - |                 - | 0x9A                            | 1
	// 1 | 1 | 0 |     - |                 - | 0xA0                            | 1
	// 1 | 0 | 1 |     - |                 - | 0xFA                            | -
	// In all other cases, the accumulator and the carry flag are left unchanged
	void CPU::applyDAA() {
		uint8_t low = reg_a & 0x0F;

		bool newcarry = flag_c;
		if (!flag_n){
			if (flag_c || reg_a > 0x99){
				reg_a += 0x60;
				newcarry = true;
			}
			if (flag_h || low > 9){
				reg_a += 0x06;
			}
		} else if (flag_c){
			newcarry = true;
			if (flag_h) reg_a += 0x9A;
			else reg_a += 0xA0;
		} else if (flag_h){
			reg_a += 0xFA;
		}

		setFlags(reg_a == 0, UNAFFECTED, 0, newcarry);
	}

	// Tell whether the emulator can skip running this component for the cycle, to save a context commutation if running it is useless
	bool CPU::skip() {
		if (m_cyclesToSkip > 0) {  // We are in-between CPU cycles (= 4 clocks)
			m_cyclesToSkip -= 1;
			return true;
		} else {
			return false;  // TODO : Skip when in STOP mode ?
		}
	}

	void CPU::logDisassembly(uint16_t position){
		std::cout << oh16(position) << " - ";

		uint8_t opcode = memoryRead(position);
		uint8_t low = memoryRead(position + 1);
		uint8_t high = memoryRead(position + 2);
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
	}

	void CPU::logStatus(){
		std::cout << "\t\taf: " << oh16(reg_af) << ", bc: " << oh16(reg_bc) << ", de: " << oh16(reg_de) << ", hl: " << oh16(reg_hl) << ", sp: " << oh16(reg_sp);
		std::cout << ", (hl): " << oh8(memoryRead(reg_hl));
		/*std::cout << "\t\tstack: ";
		for (uint16_t pointer = m_sp; pointer < 0xFFF4; pointer += 2){
			uint16_t low = m_memory->get(pointer);
			uint16_t high = m_memory->get(pointer + 1);
			uint16_t value = (high << 8) | low;
			std::cout << oh16(value) << " ";
		}*/
		std::cout << std::endl;
	}
}
