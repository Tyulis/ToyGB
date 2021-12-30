#include "debug/assembler.hpp"

#define INVALID 0xFF
#define DEFINED_OPERANDS(min, max) if (definedOperands > max || definedOperands < min) { \
										std::stringstream errstream; \
										errstream << "ERROR, line " << lineno << " : invalid amount of operands (" << definedOperands << ") for instruction " << mnemonic; \
										throw std::runtime_error(errstream.str());  \
									}

typedef struct _asm_instruction_s {
	uint8_t opcode;
	int operands;
	uint8_t operand1;
	uint8_t operand2;
	int lineno;
	bool labelRelative;
	std::string labelOperand;
} asm_instruction_t;


namespace toygb {
	const char WHITESPACE[] = " \t\n\r\f\v";

	static void trim(std::string& str) {
		size_t start = str.find_first_not_of(WHITESPACE);
		if (start == std::string::npos) {  // Only whitespace
			str.clear();
			return;
		}
		str.erase(0, start);

		size_t end = str.find_last_not_of(WHITESPACE);
		if (end == std::string::npos) {  // Only whitespace (theoretically unreachable)
			str.clear();
			return;
		}
		str.erase(end + 1);
	}

	static int decodeValue(std::string strvalue) {
		trim(strvalue);
		char typeIdentifier = strvalue.at(0);
		if (typeIdentifier == '$') {  // Hexadecimal number
			return stoi(strvalue.substr(1), nullptr, 16);
		} else if (typeIdentifier == '%') {  // Binary number
			return stoi(strvalue.substr(1), nullptr, 2);
		} else if (typeIdentifier == '0' && strvalue.size() > 1) {  // Standard base code
			char baseIdentifier = strvalue.at(1);
			if (baseIdentifier == 'x')
				return stoi(strvalue.substr(2), nullptr, 16);
			else if (baseIdentifier == 'b')
				return stoi(strvalue.substr(2), nullptr, 2);
			else if (baseIdentifier == 'd')
				return stoi(strvalue.substr(2), nullptr, 10);
			else if (baseIdentifier == 'o')
				return stoi(strvalue.substr(2), nullptr, 8);
			else
				return stoi(strvalue.substr(1), nullptr, 10);
		} else if (typeIdentifier == '-') {  // Negative number
			typeIdentifier = strvalue.at(1);
			if (typeIdentifier == '$') {  // Hexadecimal number
				return -stoi(strvalue.substr(2), nullptr, 16);
			} else if (typeIdentifier == '%') {  // Binary number
				return -stoi(strvalue.substr(2), nullptr, 2);
			} else if (typeIdentifier == '0' && strvalue.size() > 2) {  // Standard base code
				char baseIdentifier = strvalue.at(2);
				if (baseIdentifier == 'x')
					return -stoi(strvalue.substr(3), nullptr, 16);
				else if (baseIdentifier == 'b')
					return -stoi(strvalue.substr(3), nullptr, 2);
				else if (baseIdentifier == 'd')
					return -stoi(strvalue.substr(3), nullptr, 10);
				else if (baseIdentifier == 'o')
					return -stoi(strvalue.substr(3), nullptr, 8);
				else
					return -stoi(strvalue.substr(3), nullptr, 10);
			} else if (std::isdigit(typeIdentifier)) {
				return -stoi(strvalue.substr(1), nullptr, 10);
			}
		} else if (std::isdigit(typeIdentifier)) {
			return stoi(strvalue, nullptr, 10);
		}
		throw std::runtime_error("Invalid operand value " + strvalue);
	}

	static void decodeOperand(asm_instruction_t& instruction, std::string strvalue, int size, bool acceptLabel) {
		char typeIdentifier = strvalue.at(0);
		if (std::isdigit(typeIdentifier) || typeIdentifier == '$' || typeIdentifier == '%' || typeIdentifier == '-') {
			int value = decodeValue(strvalue);
			if (size == 1 && value < 0x100 && value >= -0x80) {
				instruction.operand1 = uint8_t(value);
			} else if (size == 2 && value < 0x10000 && value >= -0x8000) {
				instruction.operand1 = uint8_t(uint16_t(value) & 0xFF);  // Little endian, least significant byte first
				instruction.operand2 = uint8_t(uint16_t(value) >> 8);
			} else {
				throw std::runtime_error("Invalid operand size at " + strvalue);
			}
		} else if (size == 2 || acceptLabel) {  // Label
			instruction.labelOperand = strvalue;
			instruction.labelRelative = (size == 1);  // 1 byte = relative, 2 bytes = absolute
			return;
		} else {
			throw std::runtime_error("Invalid operand " + strvalue);
		}
	}

	static void decodeOperand(asm_instruction_t& instruction, std::string strvalue, int size) {
		decodeOperand(instruction, strvalue, size, false);
	}

	static bool isSingleRegister(std::string strvalue) {
		return strvalue == "a" || strvalue == "b" || strvalue == "c" || strvalue == "d" || strvalue == "e" || strvalue == "h" || strvalue == "l";
	}

	static uint8_t encodeSingleRegister(std::string strvalue) {
		if (strvalue == "b")      return 0b000;
		else if (strvalue == "c") return 0b001;
		else if (strvalue == "d") return 0b010;
		else if (strvalue == "e") return 0b011;
		else if (strvalue == "h") return 0b100;
		else if (strvalue == "l") return 0b101;
		else if (strvalue == "a") return 0b111;

		throw std::runtime_error("Invalid single register name " + strvalue);
	}

	static bool isDoubleRegister(std::string strvalue) {
		return strvalue == "bc" || strvalue == "de" || strvalue == "hl" || strvalue == "sp";
	}

	static uint8_t encodeDoubleRegister(std::string strvalue) {
		if (strvalue == "bc")      return 0b00;
		else if (strvalue == "de") return 0b01;
		else if (strvalue == "hl") return 0b10;
		else if (strvalue == "sp") return 0b11;
		else if (strvalue == "af") return 0b11;

		throw std::runtime_error("Invalid double register name " + strvalue);
	}

	static uint8_t encodeCondition(std::string strvalue) {
		if (strvalue == "nz")      return 0b00;
		else if (strvalue == "z")  return 0b01;
		else if (strvalue == "nc") return 0b10;
		else if (strvalue == "c")  return 0b11;

		throw std::runtime_error("Invalid condition " + strvalue);
	}

	static bool isReference(std::string strvalue) {
		return (strvalue.front() == '(' && strvalue.back() == ')') || (strvalue.front() == '[' && strvalue.back() == ']');
	}

	static std::string getReference(std::string strvalue) {
		trim(strvalue);
		size_t start = strvalue.find_first_not_of("([");
		size_t end = strvalue.find_last_not_of(")]");
		if (start == std::string::npos || end == std::string::npos)
			return "";
		std::string referenced = strvalue.substr(start, end-start + 1);
		trim(referenced);
		return referenced;
	}

	static uint8_t encodeArithmeticalOperation(std::string mnemonic) {
		if (mnemonic == "add")      return 0b000;
		else if (mnemonic == "adc") return 0b001;
		else if (mnemonic == "sub") return 0b010;
		else if (mnemonic == "sbc") return 0b011;
		else if (mnemonic == "and") return 0b100;
		else if (mnemonic == "xor") return 0b101;
		else if (mnemonic == "or")  return 0b110;
		else if (mnemonic == "cp")  return 0b111;

		throw std::runtime_error("Invalid arithmetical operation " + mnemonic);
	}

	static uint8_t encodeShiftRotateOperation(std::string mnemonic) {
		if (mnemonic == "rlc")       return 0b000;
		else if (mnemonic == "rrc")  return 0b001;
		else if (mnemonic == "rl")   return 0b010;
		else if (mnemonic == "rr")   return 0b011;
		else if (mnemonic == "sla")  return 0b100;
		else if (mnemonic == "sra")  return 0b101;
		else if (mnemonic == "swap") return 0b110;
		else if (mnemonic == "srl")  return 0b111;

		throw std::runtime_error("Invalid shift/rotate operation " + mnemonic);
	}

	static uint8_t encodeBitOperation(std::string mnemonic) {
		if (mnemonic == "bit")       return 0b01;
		else if (mnemonic == "res")  return 0b10;
		else if (mnemonic == "set")  return 0b11;

		throw std::runtime_error("Invalid shift/rotate operation " + mnemonic);
	}

	static asm_instruction_t buildInstruction(std::string mnemonic, int definedOperands, std::string operand1, std::string operand2, int lineno) {
		asm_instruction_t instruction = {.opcode = 0xFF, .operands = -1, .operand1 = 0xFF, .operand2 = 0xFF, .lineno = lineno, .labelRelative = false, .labelOperand = ""};

		// Well...

		// 00 000000 | 0x00 | nop | Do nothing for a cycle | 1 | ----
		if (mnemonic == "nop") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0x00;
			instruction.operands = 0;
		}

		// 00 01 0000 | 0x10 | stop | Stop the clock to get into a very low-power mode (or to switch to CGB double-speed mode) | 2 | ----
		else if (mnemonic == "stop") {  DEFINED_OPERANDS(0, 1)
			instruction.opcode = 0x10;
			instruction.operands = 1;
			if (definedOperands == 0)
				instruction.operand1 = 0x00;
			else  // In case anyone wants to assemble a corrupted stop
				decodeOperand(instruction, operand1, 1);
		}

		// 00 rrr 110 | 0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x3E | ld r, u8                  | Load an immediate value into a register                                           | 2 | ----
		// 00 rr 1010 | 0x0A, 0x1A                               | ld a, (rr)                | Load the value at the address given by the value of a 16-bits register into A     | 2 | ----
		// 00 10 1010 | 0x2A                                     | ld a, (hli) / ld a, (hl+) | Load the value at the address given by HL into register A, then increment HL by 1 | 2 | ----
		// 00 11 1010 | 0x3A                                     | ld a, (hld) / ld a, (hl-) | Load the value at the address given by HL into register A, then decrement HL by 1 | 2 | ----
		// 11 11 0000 | 0xF0                                     | ld a, ($FF00+u8)          | Load the value at memory address 0xFF00 + u8 into register A                      | 3 | ----
		// 11 11 0010 | 0xF2                                     | ld a, ($FF00+c)           | Load the value at address (0xFF00 + value of the register C) into the register A  | 2 | ----
		// 11 11 1010 | 0xFA                                     | ld a, (u16)               | Load the value at an immediate memory address into register A                     | 4 | ----
		// 01 rrr 110 | 0x46, 0x4E, 0x56, 0x5E, 0x66, 0x6E, 0x7E | ld r, (hl)                | Load the value at the memory address given by HL into a register                  | 2 | ----
		// 01 xxx yyy | All other values in 0x40-0x7F            | ld x, y                   | Load the value of register y into register x                                      | 1 | ----

		// 00 rr 0001 | 0x01, 0x11, 0x21, 0x31                   | ld rr, u16                | Load an immediate 16-bits value into a 16-bits register                           | 3 | ----
		// 11 11 1000 | 0xF8                                     | ld hl, sp+s8              | Load the value of (SP + signed 8-bits immediate value) into HL                    | 3 | 00hc
		// 11 11 1001 | 0xF9                                     | ld sp, hl                 | Load the value of HL into SP                                                      | 2 | ----

		// 01 110 rrr | 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x77 | ld (hl), r                | Load the value of a register into memory at the address given by HL               | 2 | ----
		// 00 110 110 | 0x36                                     | ld (hl), u8               | Load an immediate value into the memory address given by HL                       | 3 | ----
		// 00 10 0010 | 0x22                                     | ld (hli), a / ld (hl+), a | Load the value of A into the memory address given by HL, then increment HL by 1   | 2 | ----
		// 00 11 0010 | 0x32                                     | ld (hld), a / ld (hl-), a | Load the value of A into the memory address given by HL, then decrement HL by 1   | 2 | ----
		// 00 rr 0010 | 0x02, 0x12                               | ld (rr), a                | Load the value of A into the memory address given by BC or DE                     | 2 | ----
		// 00 00 1000 | 0x08                                     | ld (u16), sp              | Load the value of SP into a 16-bits immediate address                             | 5 | ----
		// 11 10 1010 | 0xEA                                     | ld (u16), a               | Load the value of a into an immediate memory address                              | 4 | ----
		// 11 10 0010 | 0xE2                                     | ld ($FF00+c), a           | Load the value of A into the address (0xFF00 + value of the register C)           | 2 | ----
		// 11 10 0000 | 0xE0                                     | ld ($FF00+u8), a          | Load the value of register A into memory at address 0xFF00 + u8                   | 3 | ----
		else if (mnemonic == "ld") {  DEFINED_OPERANDS(2, 2)
			if (isSingleRegister(operand1)) {
				// ld x, y
				if (isSingleRegister(operand2)) {
					instruction.opcode = 0x40 | (encodeSingleRegister(operand1) << 3) | encodeSingleRegister(operand2);
					instruction.operands = 0;
				} else if (isReference(operand2)) {
					std::string referenced = getReference(operand2);
					if (operand1 == "a" && (referenced.substr(0, 5) == "$FF00" || referenced.substr(0, 5) == "$ff00")) {
						std::string highvalue = referenced.substr(referenced.find_first_of('+') + 1);
						trim(highvalue);
						// ld a, ($FF00+c)
						if (highvalue == "c") {
							instruction.opcode = 0xF2;
							instruction.operands = 0;
						}
						// ld a, ($FF00+u8)
						else {
							instruction.opcode = 0xF0;
							instruction.operands = 1;
							decodeOperand(instruction, highvalue, 1);
						}
					}

					// ld a, (hl+) / ld a, (hli)
					else if (operand1 == "a" && (referenced == "hli" || referenced == "hl+")) {
						instruction.opcode = 0x2A;
						instruction.operands = 0;
					}

					// ld a, (hl-) / ld a, (hld)
					else if (operand1 == "a" && (referenced == "hld" || referenced == "hl-")) {
						instruction.opcode = 0x3A;
						instruction.operands = 0;
					}

					// ld r, (hl)
					else if (referenced == "hl") {
						instruction.opcode = 0x46 | (encodeSingleRegister(operand1) << 3);
						instruction.operands = 0;
					}

					// ld a, (rr)
					else if (operand1 == "a" && (referenced == "bc" || referenced == "de")) {
						instruction.opcode = 0x0A | (encodeDoubleRegister(referenced) << 4);
						instruction.operands = 0;
					}

					// ld a, (u16)
					else if (operand1 == "a") {
						instruction.opcode = 0xFA;
						instruction.operands = 2;
						decodeOperand(instruction, referenced, 2);
					}
				}

				// ld r, u8
				else {
					instruction.opcode = 0x06 | (encodeSingleRegister(operand1) << 3);
					instruction.operands = 1;
					decodeOperand(instruction, operand2, 1);
				}
			} else if (isDoubleRegister(operand1)) {
				// ld sp, hl
				if (operand1 == "sp" && operand2 == "hl") {
					instruction.opcode = 0xF9;
					instruction.operands = 0;
				}

				// ld hl, sp+s8
				else if (operand1 == "hl" && operand2.substr(0, 2) == "sp") {
					instruction.opcode = 0xF8;
					instruction.operands = 1;
					size_t pluspos = operand2.find_first_of('+');
					if (pluspos == std::string::npos) {
						instruction.operand1 = 0x00;
					} else {
						std::string disp = operand2.substr(pluspos + 1);
						trim(disp);
						decodeOperand(instruction, disp, 1);
					}
				}

				// ld rr, u16
				else {
					instruction.opcode = 0x01 | (encodeDoubleRegister(operand1) << 4);
					instruction.operands = 2;
					decodeOperand(instruction, operand2, 2);
				}
			} else if (isReference(operand1)) {
				std::string referenced = getReference(operand1);
				if (referenced == "hl") {
					// ld (hl), r
					if (isSingleRegister(operand2)) {
						instruction.opcode = 0x70 | encodeSingleRegister(operand2);
						instruction.operands = 0;
					}
					// ld (hl), u8
					else {
						instruction.opcode = 0x36;
						instruction.operands = 1;
						decodeOperand(instruction, operand2, 1);
					}
				}

				// ld (hl+), a / ld (hli), a
				else if (operand2 == "a" && (referenced == "hli" || referenced == "hl+")) {
					instruction.opcode = 0x22;
					instruction.operands = 0;
				}

				// ld (hl-), a / ld (hld), a
				else if (operand2 == "a" && (referenced == "hld" || referenced == "hl-")) {
					instruction.opcode = 0x32;
					instruction.operands = 0;
				}

				// ld (rr), a
				else if (operand2 == "a" && (referenced == "bc" || referenced == "de")) {
					instruction.opcode = 0x02 | (encodeDoubleRegister(referenced) << 4);
					instruction.operands = 0;
				}

				else if (operand2 == "a" && (referenced.substr(0, 5) == "$FF00" || referenced.substr(0, 5) == "$ff00")) {
					std::string highvalue = referenced.substr(referenced.find_first_of('+') + 1);
					trim(highvalue);
					// ld ($FF00+c), a
					if (highvalue == "c") {
						instruction.opcode = 0xE2;
						instruction.operands = 0;
					}
					// ld ($FF00+u8), a
					else {
						instruction.opcode = 0xE0;
						instruction.operands = 1;
						decodeOperand(instruction, highvalue, 1);
					}
				}

				// ld (u16), a
				else if (operand2 == "a") {
					instruction.opcode = 0xEA;
					instruction.operands = 2;
					decodeOperand(instruction, referenced, 2);
				}

				// ld (u16), sp
				else if (operand2 == "sp") {
					instruction.opcode = 0x08;
					instruction.operands = 2;
					decodeOperand(instruction, referenced, 2);
				}
			}
		}

		// 00 10 0010 | 0x22 | ldi (hl), a / ld (hl+), a | Load the value of A into the memory address given by HL, then increment HL by 1   | 2 | ----
		// 00 10 1010 | 0x2A | ldi a, (hl) / ld a, (hl+) | Load the value at the address given by HL into register A, then increment HL by 1 | 2 | ----
		else if (mnemonic == "ldi") {  DEFINED_OPERANDS(2, 2)
			if (isReference(operand1)) {
				std::string referenced = getReference(operand1);
				// ldi (hl), a
				if (referenced == "hl" && operand2 == "a") {
					instruction.opcode = 0x22;
					instruction.operands = 0;
				}
			} else if (isReference(operand2)) {
				std::string referenced = getReference(operand2);
				//ldi a, (hl)
				if (operand1 == "a" && referenced == "hl") {
					instruction.opcode = 0x2A;
					instruction.operands = 0;
				}
			}
		}

		// 00 11 0010 | 0x32 | ldd (hl, a) / ld (hl-), a | Load the value of A into the memory address given by HL, then decrement HL by 1 | 2 | ----
		// 00 11 1010 | 0x3A | ldd a, (hl) / ld a, (hl-) | Load the value at the address given by HL into register A, then decrement HL by 1 | 2 | ----
		else if (mnemonic == "ldd") {  DEFINED_OPERANDS(2, 2)
			if (isReference(operand1)) {
				std::string referenced = getReference(operand1);
				// ldd (hl), a
				if (referenced == "hl" && operand2 == "a") {
					instruction.opcode = 0x32;
					instruction.operands = 0;
				}
			} else if (isReference(operand2)) {
				std::string referenced = getReference(operand2);
				//ldd a, (hl)
				if (operand1 == "a" && referenced == "hl") {
					instruction.opcode = 0x3A;
					instruction.operands = 0;
				}
			}
		}

		// 11 10 0000 | 0xE0 | ldh (u8), a | Load the value of register A into memory at address 0xFF00 + u8                  | 3 | ----
		// 11 11 0000 | 0xF0 | ldh a, (u8) | Load the value at memory address 0xFF00 + u8 into register A                     | 3 | ----
		// 11 10 0010 | 0xE2 | ldh (c), a  | Load the value of A into the address (0xFF00 + value of the register C)          | 2 | ----
		// 11 11 0010 | 0xF2 | ldh a, (c)  | Load the value at address (0xFF00 + value of the register C) into the register A | 2 | ----
		else if (mnemonic == "ldh") {  DEFINED_OPERANDS(2, 2)
			if (isReference(operand1) && operand2 == "a") {
				std::string referenced = getReference(operand1);
				// ldh (c), a
				if (referenced == "c") {
					instruction.opcode = 0xE2;
					instruction.operands = 0;
				}
				// ldh (u8), a
				else {
					instruction.opcode = 0xE0;
					int value = decodeValue(referenced);
					if (value >= 0xFF00 && value <= 0xFFFF) {
						instruction.operand1 = value & 0xFF;
					} else if (value >= 0x00 && value <= 0xFF) {
						instruction.operand1 = value;
					} else {
						throw std::runtime_error("Invalid high address " + referenced);
					}
					instruction.operands = 1;
				}
			} else if (operand1 == "a" && isReference(operand2)) {
				std::string referenced = getReference(operand2);
				// ldh a, (c)
				if (referenced == "c") {
					instruction.opcode = 0xF2;
					instruction.operands = 0;
				}
				// ldh a, (u8)
				else {
					instruction.opcode = 0xF0;
					int value = decodeValue(referenced);
					if (value >= 0xFF00 && value <= 0xFFFF) {
						instruction.operand1 = value & 0xFF;
					} else if (value >= 0x00 && value <= 0xFF) {
						instruction.operand1 = value;
					} else {
						throw std::runtime_error("Invalid high address " + referenced);
					}
					instruction.operands = 1;
				}
			}
		}

		// 00 rrr 100 | 0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x3C | inc r    | Increment the value of a register by 1                     | 1 | z0h-
		// 00 110 100 | 0x34                                     | inc (hl) | Increment the value at the memory address given by HL by 1 | 3 | z0h-
		// 00 rr 0011 | 0x03, 0x13, 0x23, 0x33                   | inc rr   | Increment the value of a 16-bits register by 1             | 2 | ----
		else if (mnemonic == "inc") {  DEFINED_OPERANDS(1, 1)
			// inc r
			if (isSingleRegister(operand1)) {
				instruction.opcode = 0x04 | (encodeSingleRegister(operand1) << 3);
				instruction.operands = 0;
			}
			// inc (hl)
			else if (isReference(operand1)) {
				if (getReference(operand1) == "hl") {
					instruction.opcode = 0x34;
					instruction.operands = 0;
				}
			}
			// inc rr
			else if (isDoubleRegister(operand1)) {
				instruction.opcode = 0x03 | (encodeDoubleRegister(operand1) << 4);
				instruction.operands = 0;
			}
		}

		// 00 rrr 101 | 0x05, 0x0D, 0x15, 0x1D, 0x25, 0x2D, 0x3D | dec r    | Decrement the value of a register by 1                     | 1 | z1h-
		// 00 110 101 | 0x35                                     | dec (hl) | Decrement the value at the memory address given by HL by 1 | 3 | z1h-
		// 00 00 1011 | 0x0B, 0x1B, 0x2B, 0x3B                   | dec rr   | Decrement the value of BC by 1                             | 2 | ----
		else if (mnemonic == "dec") {  DEFINED_OPERANDS(1, 1)
			// dec r
			if (isSingleRegister(operand1)) {
				instruction.opcode = 0x05 | (encodeSingleRegister(operand1) << 3);
				instruction.operands = 0;
			}
			// dec (hl)
			else if (isReference(operand1)) {
				if (getReference(operand1) == "hl") {
					instruction.opcode = 0x35;
					instruction.operands = 0;
				}
			}
			// dec rr
			else if (isDoubleRegister(operand1)) {
				instruction.opcode = 0x0B | (encodeDoubleRegister(operand1) << 4);
				instruction.operands = 0;
			}
		}

		// 00 00 0111 | 0x07 | rlca | Rotate the accumulator's bits left (c 76543210 -> 7 65432107) | 1 | 000c
		else if (mnemonic == "rlca") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0x07;
			instruction.operands = 0;
		}

		// 00 01 0111 | 0x17 | rla | Rotate the accumulator and carry bits left (c 76543210 -> 7 6543210c) | 1 | 000c
		else if (mnemonic == "rla") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0x17;
			instruction.operands = 0;
		}

		// 00 10 0111 | 0x27 | daa | For Binary-Coded Decimal value (e.g 0x75 for the decimal value 75), adjust the value back to BCD after an arithmetical operation with another BCD operands | 1 | z-0c
		//                         | Example (decimal : 75 + 19 = 94) : 0x75 + 0x19 = 0x8E -- daa -> 0x94
		else if (mnemonic == "daa") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0x27;
			instruction.operands = 0;
		}

		// 00 11 0111 | 0x37 | scf | Set the carry flag | 1 | -001
		else if (mnemonic == "scf") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0x37;
			instruction.operands = 0;
		}

		// 00 00 1111 | 0x0F | rrca | Rotate the accumulator's bits right (76543210 c -> 07654321 0) | 1 | 000c
		else if (mnemonic == "rrca") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0x0F;
			instruction.operands = 0;
		}

		// 00 01 1111 | 0x1F | rra | Rotate the accumulator's and carry bits right (76543210 c -> c7654321 0) | 1 | 000c
		else if (mnemonic == "rra") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0x1F;
			instruction.operands = 0;
		}

		// 00 10 1111 | 0x2F | cpl | Take the complement of the accumulator (flip all bits) | 1 | -11-
		else if (mnemonic == "cpl") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0x2F;
			instruction.operands = 0;
		}

		// 00 11 1111 | 0x3F | ccf | Take the complement of the carry flag (flip flag c) | 1 | -00c
		else if (mnemonic == "ccf") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0x3F;
			instruction.operands = 0;
		}

		// 01 110 110 | 0x76 | halt | Put the CPU in halt mode (low-power mode where it does nothing) until an interrupt is requested and enabled (IE + IF, not necessarily IME) | 1 | ----
		else if (mnemonic == "halt") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0x76;
			instruction.operands = 0;
		}

		// 10 000 rrr | 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x87 | add a, r    | Add the value of a register to the accumulator              | 1 | z0hc
		// 10 000 110 | 0x86                                     | add a, (hl) | Add the value at the address given by HL to the accumulator | 2 | z0hc
		// 11 000 110 | 0xC6                                     | add a, u8   | Add an immediate value to the accumulator                   | 2 | z0hc
		// 00 rr 1001 | 0x09, 0x19, 0x29, 0x39                   | add hl, rr  | Add the value of a 16-register to HL                        | 2 | -0hc
		// 11 10 1000 | 0xE8                                     | add sp, s8  | Add a signed 8-bits immediate value to the value of SP      | 4 | 00hc
		else if (mnemonic == "add") {  DEFINED_OPERANDS(1, 2)
			if ((definedOperands == 2 && operand1 == "a") || definedOperands == 1) {
				std::string operand = (definedOperands == 2 ? operand2 : operand1);
				// add a, r / add r
				if (isSingleRegister(operand)) {
					instruction.opcode = 0x80 | encodeSingleRegister(operand);
					instruction.operands = 0;
				}
				// add a, (hl) / add (hl)
				else if (isReference(operand)) {
					if (getReference(operand) == "hl") {
						instruction.opcode = 0x86;
						instruction.operands = 0;
					}
				}
				// add a, u8 / add u8
				else {
					instruction.opcode = 0xC6;
					instruction.operands = 1;
					decodeOperand(instruction, operand, 1);
				}
			} else {
				// add hl, rr
				if (operand1 == "hl" && isDoubleRegister(operand2)) {
					instruction.opcode = 0x09 | (encodeDoubleRegister(operand2) << 4);
					instruction.operands = 0;
				}
				// add sp, s8
				else if (operand1 == "sp") {
					instruction.opcode = 0xE8;
					instruction.operands = 1;
					decodeOperand(instruction, operand2, 1);
				}
			}
		}

		// 10 ppp rrr | All other values in 0x80-0xBF                  | <op> a, r    | Do an arithmetical operation between the accumulator and another register and put the result back in the accumulator                               | 1 | xxxx
		// 10 ppp 110 | 0x86, 0x8E, 0x96, 0x9E, 0xA6, 0xAE, 0xB6, 0xBE | <op> a, (hl) | Do an arithmetical operation between the accumulator and the value in memory at the address given by HL and put the result back in the accumulator | 2 | xxxx
		// 11 ppp 110 | 0xC6, 0xCE, 0xD6, 0xDE, 0xE6, 0xEE, 0xF6, 0xFE | <op> a, u8   | Perform an arithmetical operation between the accumulator and an immediate value, and put the result back into the accumulator                     | 2 | xxxx
		else if (mnemonic == "adc" || mnemonic == "sub" || mnemonic == "sbc" || mnemonic == "and" || mnemonic == "or" || mnemonic == "xor" || mnemonic == "cp") {  DEFINED_OPERANDS(1, 2)
			if ((definedOperands == 2 && operand1 == "a") || definedOperands == 1) {
				std::string operand = (definedOperands == 2 ? operand2 : operand1);
				// <op> a, r / <op> r
				if (isSingleRegister(operand)) {
					instruction.opcode = 0x80 | (encodeArithmeticalOperation(mnemonic) << 3) | encodeSingleRegister(operand);
					instruction.operands = 0;
				}
				// <op> a, (hl) / <op> (hl)
				else if (isReference(operand)) {
					if (getReference(operand) == "hl") {
						instruction.opcode = 0x86 | (encodeArithmeticalOperation(mnemonic) << 3);
						instruction.operands = 0;
					}
				}
				// <op> a, u8 / <op> u8
				else {
					instruction.opcode = 0xC6 | (encodeArithmeticalOperation(mnemonic) << 3);
					instruction.operands = 1;
					decodeOperand(instruction, operand, 1);
				}
			}
		}

		// 11 rr 0001 | 0xC1, 0xD1, 0xE1 | pop rr | Pop a 16-bits value from the stack into a 16-bits register                                             | 3 | ----
		// 11 11 0001 | 0xF1             | pop af | Pop a 16-bits value from the stack into 16-bits register AF (that replaces SP as identifier 0b11 here) | 3 | znhc
		else if (mnemonic == "pop") {  DEFINED_OPERANDS(1, 1)
			if (operand1 == "bc" || operand1 == "de" || operand1 == "hl" || operand1 == "af") {
				instruction.opcode = 0xC1 | (encodeDoubleRegister(operand1) << 4);
				instruction.operands = 0;
			}
		}

		// 11 rr 0101 | 0xC5, 0xD5, 0xE5 | push rr | Push the value of a 16-bits register onto the stack      | 4 | ----
		// 11 11 0101 | 0xF5             | push af | Push the value of the 16-bits register AF onto the stack | 4 | ----
		else if (mnemonic == "push") {  DEFINED_OPERANDS(1, 1)
			if (operand1 == "bc" || operand1 == "de" || operand1 == "hl" || operand1 == "af") {
				instruction.opcode = 0xC5 | (encodeDoubleRegister(operand1) << 4);
				instruction.operands = 0;
			}
		}

		// 001 cc 000 | 0x20, 0x28, 0x30, 0x38 | jr [nz, z, nc, c], s8 | Conditional relative jump, by a number of bytes given by the given signed value             | 3 (jump) / 2 (condition false, no jump) | ----
		// 00 01 1000 | 0x18                   | jr s8                 | Unconditional relative jump, by a number of bytes given by an immediate signed displacement | 3                                       | ----
		else if (mnemonic == "jr") {  DEFINED_OPERANDS(1, 2)
			// jr s8
			if (definedOperands == 1) {
				instruction.opcode = 0x18;
				instruction.operands = 1;
				decodeOperand(instruction, operand1, 1, true);
			}
			// jr cc, s8
			else {
				instruction.opcode = 0x20 | (encodeCondition(operand1) << 3);
				instruction.operands = 1;
				decodeOperand(instruction, operand2, 1, true);
			}
		}

		// 110 cc 010 | 0xC2, 0xCA, 0xD2, 0xDA | jp [nz, z, nc, c], u16 | Conditional absolute jump, jump to an immediate 16-bits address if the condition is true | 4 (jump, condition true) / 3 (false) | ----
		// 11 00 0011 | 0xC3                   | jp u16                 | Unconditional absolute jump to an immediate 16-bits address                              | 4                                    | ----
		// 11 10 1001 | 0xE9                   | jp hl                  | Unconditonal jump to the address given by HL | 1 | ----
		else if (mnemonic == "jp") {  DEFINED_OPERANDS(1, 2)
			if (definedOperands == 1) {
				// jp hl
				if (operand1 == "hl") {
					instruction.opcode = 0xE9;
					instruction.operands = 0;
				}
				// jp u16
				else {
					instruction.opcode = 0xC3;
					instruction.operands = 2;
					decodeOperand(instruction, operand1, 2);
				}
			}
			// jp cc, u16
			else {
				instruction.opcode = 0xC2 | (encodeCondition(operand1) << 3);
				instruction.operands = 2;
				decodeOperand(instruction, operand2, 2);
			}
		}

		// 110 cc 100 | 0xC4, 0xCC, 0xD4, 0xDC | call [nz, z, nc, c], u16 | Conditonally call a subroutine at an immediate 16-bits address. If the condition is true, PC is pushed on the stack then jumps | 6 (call, condition true) / 3 (false) | ----
		// 11 00 1101 | 0xCD                   | call u16                 | Unconditionally call a subroutine at an immediate address                                                                      | 6                                    | ----
		else if (mnemonic == "call") {  DEFINED_OPERANDS(1, 2)
			// call u16
			if (definedOperands == 1) {
				instruction.opcode = 0xCD;
				instruction.operands = 2;
				decodeOperand(instruction, operand1, 2);
			}
			// call cc, u16
			else {
				instruction.opcode = 0xC4 | (encodeCondition(operand1) << 3);
				instruction.operands = 2;
				decodeOperand(instruction, operand2, 2);
			}
		}

		// 11 xxx 111 | 0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF | rst xx | Call a reset vector (0x0000 / 0x0008 / 0x0010 / 0x0018 / 0x0020 / 0x0028 / 0x0030 / 0x0038) | 4 | ----
		else if (mnemonic == "rst") {  DEFINED_OPERANDS(1, 1)
			int value = decodeValue(operand1);
			if ((value & 0xC7) != 0x00)
				throw std::runtime_error("Invalid reset vector : " + operand1);
			instruction.opcode = 0xC7 | value;
			instruction.operands = 0;
		}

		// 11 00 1001 | 0xC9                   | ret                | Unconditionally return from a subroutine                                        | 4                                      | ----
		// 110 cc 000 | 0xC0, 0xC8, 0xD0, 0xD8 | ret [nz, z, nc, c] | Conditional return, pop the value of PC from the stack if the condition is true | 5 (return, condition true) / 2 (false) | ----
		else if (mnemonic == "ret") {  DEFINED_OPERANDS(0, 1)
			instruction.operands = 0;
			if (definedOperands == 0)
				instruction.opcode = 0xC9;
			else
				instruction.opcode = 0xC0 | (encodeCondition(operand1) << 3);
		}

		// 11 01 1001 | 0xD9 | reti | Unconditionally return from a subroutine and enable interrupts (set IME) | 4 | ----
		else if (mnemonic == "reti") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0xD9;
			instruction.operands = 0;
		}

		// 11 11 0011 | 0xF3 | di | Immediately clear IME (Interrupts Master Enable) : Until ei or reti are executed, requested and enabled interrupts stay pending and to not trigger a jump to the interrupt vector | 1 | ----
		else if (mnemonic == "di") {  DEFINED_OPERANDS(0, 0)
			instruction.opcode = 0xF3;
			instruction.operands = 0;
		}

		// 11 11 1011 | 0xFB | ei | Enable interrupts (set the Interrupt Master Enable), with a delay of 1 cycle | 1 | ----
		else if (mnemonic == "ei") {
			instruction.opcode = 0xFB;
			instruction.operands = 0;
		}

		// 11 00 1011            | 0xCB           | Prefix for bitwise operations, specific opcode is the next byte | 2 / 4 (with (hl))                                   | xxxx
		//            00 000 rrr |      0x00-0x07 | rlc r  | Rotate the bits of a register left (c 76543210 -> 7 65432107)                                                | z00c
		//            00 001 rrr |      0x08-0x0F | rrc r  | Rotate the bits of a register right (76543210 c -> 07654321 0)                                               | z00c
		//            00 010 rrr |      0x10-0x17 | rl r   | Rotate the bits of a register and carry left (c 76543210 -> 7 6543210c)                                      | z00c
		//            00 011 rrr |      0x18-0x1F | rr r   | Rotate the bits of a register and carry right (76543210 c -> c7654321 0)                                     | z00c
		//            00 100 rrr |      0x20-0x27 | sla r  | Shift the bits of a register left (c mnopqrst -> m nopqrst0)                                                 | z00c
		//            00 101 rrr |      0x28-0x2F | sra r  | Shift the bits of a register right, leaving the leftmost bit at its initial value (mnopqrst c -> mmnopqrs t) | z00c
		//            00 110 rrr |      0x30-0x37 | swap r | Swap the upper and lower nibbles of a register (76543210 -> 32107654)                                        | z000
		//            00 111 rrr |      0x38-0x3F | srl l  | Shift the bits of a register right, leaving zero in the leftmost bit (mnopqrst c -> 0mnopqrs t)              | z00c
		else if (mnemonic == "rlc" || mnemonic == "rrc" || mnemonic == "rl" || mnemonic == "rr" || mnemonic == "sla" || mnemonic == "sra" || mnemonic == "swap" || mnemonic == "srl") {  DEFINED_OPERANDS(1, 1)
			instruction.opcode = 0xCB;
			// <op> r
			if (isSingleRegister(operand1)) {
				instruction.operand1 = 0x00 | (encodeShiftRotateOperation(mnemonic) << 3) | encodeSingleRegister(operand1);
				instruction.operands = 1;
			}
			// <op> (hl)
			else if (isReference(operand1)) {
				if (getReference(operand1) == "hl") {
					instruction.operand1 = 0x06 | (encodeShiftRotateOperation(mnemonic) << 3);
					instruction.operands = 1;
				}
			}
		}

		// 11 00 1011            | 0xCB                         | Prefix for bitwise operations, specific opcode is the next byte | 2 / 4 (with (hl))                          | xxxx
		//            01 bbb rrr |      All values in 0x40-0x7F | bit b, r | Check the value of bit b of the value of a register. Bit = 0 -> flag z = 1, bit = 1 -> flag z = 0 | z01-
		//            10 bbb rrr |      All values in 0x80-0xBF | res b, r | Reset (set to 0) bit b of the value of a register                                                 | ----
		//            11 bbb rrr |      All values in 0xC0-0xFF | set b, r | Set (to 1) bit b of the value of a register                                                       | ----
		else if (mnemonic == "bit" || mnemonic == "set" || mnemonic == "res") {  DEFINED_OPERANDS(2, 2)
			instruction.opcode = 0xCB;
			int bit = decodeValue(operand1);
			// <op> b, r
			if (isSingleRegister(operand2)) {
				instruction.operand1 = 0x00 | (encodeBitOperation(mnemonic) << 6) | (bit << 3) | encodeSingleRegister(operand2);
				instruction.operands = 1;
			}
			// <op> b, (hl)
			else if (isReference(operand2)) {
				if (getReference(operand2) == "hl") {
					instruction.operand1 = 0x06 | (encodeBitOperation(mnemonic) << 6) | (bit << 3);
					instruction.operands = 1;
				}
			}
		}

		if (instruction.operands < 0) {
			throw std::runtime_error("Invalid instruction " + mnemonic + (operand1 != "" ? " " + operand1 : "") + (operand2 != "" ? ", " + operand2 : ""));
		}
		return instruction;
	}

	// Quick-and-dirty assembler to assemble simple code on-the-fly
	std::vector<uint8_t> assemble(std::string code) {
		std::stringstream codestream(code);
		std::string line = "";
		std::vector<asm_instruction_t> instructions;  // Intermediate instruction list
		std::map<std::string, int> labels;        // Associate label names to their position
		std::string lastLabel = "";
		int position = 0;
		int lineno = 0;
		int errors = 0;

		// Translate instructions
		while (std::getline(codestream, line, '\n')) {
			lineno += 1;

			// Delete comments
			size_t commentStart = line.find_first_of(';');
			if (commentStart != std::string::npos)
				line.erase(commentStart);

			// Trim leading and trailing whitespaces
			trim(line);
			if (line.empty())
				continue;

			// Convert everything to lowercase
			std::transform(line.begin(), line.end(), line.begin(),
				[](unsigned char c){ return std::tolower(c); });

			size_t colonPosition = line.find_first_of(':');
			if (colonPosition == std::string::npos) {  // Does not contain : = instruction
				size_t separator = line.find_first_of(WHITESPACE);
				int definedOperands = 0;
				std::string mnemonic = "", operand1 = "", operand2 = "";
				if (separator == std::string::npos) {  // No operands
					mnemonic = line;
					definedOperands = 0;
				} else {
					mnemonic = line.substr(0, separator);
					std::string operands = line.substr(separator + 1);
					size_t comma = operands.find_first_of(',');
					if (comma == std::string::npos) {  // No comma = 1 operand
						operand1 = operands;
						definedOperands = 1;
					} else {
						operand1 = operands.substr(0, comma);
						operand2 = operands.substr(comma + 1);
						definedOperands = 2;
					}
				}

				trim(mnemonic); trim(operand1); trim(operand2);

				try {
					// ASM DIRECTIVE .db : Declare raw bytes
					if (mnemonic == ".db") {
						if (operand1.empty()) {
							throw std::runtime_error("No argument given to .db");
						} else {
							std::string operands = line.substr(separator + 1);
							size_t comma = operands.find_first_of(',');
							while (comma != std::string::npos) {
								int value = decodeValue(operands.substr(0, comma));
								if (value < -0x80 || value > 0xFF)
									throw std::runtime_error("Byte value out of range " + operands.substr(0, comma));
								operands = operands.substr(comma + 1);
								comma = operands.find_first_of(',');

								asm_instruction_t instruction = {.opcode = uint8_t(value), .operands = 0, .operand1 = 0xFF, .operand2 = 0xFF, .lineno = lineno, .labelRelative = false, .labelOperand = ""};
								instructions.push_back(instruction);
								position += 1;
							}

							int value = decodeValue(operands);
							if (value < -0x80 || value > 0xFF)
								throw std::runtime_error("Byte value out of range " + operands.substr(0, comma));

							asm_instruction_t instruction = {.opcode = uint8_t(value), .operands = 0, .operand1 = 0xFF, .operand2 = 0xFF, .lineno = lineno, .labelRelative = false, .labelOperand = ""};
							instructions.push_back(instruction);
							position += 1;
						}
					}

					// ASM DIRECTIVE .dw/.ds : Declare raw 16-bits values
					else if (mnemonic == ".ds" || mnemonic == ".dw") {
						if (operand1.empty()) {
							throw std::runtime_error("No argument given to " + mnemonic);
						} else {
							std::string operands = line.substr(separator + 1);
							size_t comma = operands.find_first_of(',');
							while (comma != std::string::npos) {
								int value = decodeValue(operands.substr(0, comma));
								if (value < -0x8000 || value > 0xFFFF)
									throw std::runtime_error("Word value out of range " + operands.substr(0, comma));
								operands = operands.substr(comma + 1);
								comma = operands.find_first_of(',');

								asm_instruction_t instruction = {.opcode = uint8_t(uint16_t(value) & 0xFF), .operands = 1, .operand1 = uint8_t(uint16_t(value) >> 8), .operand2 = 0xFF, .lineno = lineno, .labelRelative = false, .labelOperand = ""};
								instructions.push_back(instruction);
								position += 2;
							}

							int value = decodeValue(operands);
							if (value < -0x8000 || value > 0xFFFF)
								throw std::runtime_error("Word value out of range " + operands.substr(0, comma));

							asm_instruction_t instruction = {.opcode = uint8_t(uint16_t(value) & 0xFF), .operands = 1, .operand1 = uint8_t(uint16_t(value) >> 8), .operand2 = 0xFF, .lineno = lineno, .labelRelative = false, .labelOperand = ""};
							instructions.push_back(instruction);
							position += 2;
						}
					}

					// Standard instruction
					else {
						asm_instruction_t instruction = buildInstruction(mnemonic, definedOperands, operand1, operand2, lineno);
						instructions.push_back(instruction);
						position += instruction.operands + 1;
					}
				} catch (std::exception const& exc) {
					errors += 1;
					std::cerr << "ERROR, line " << lineno << " : " << exc.what() << std::endl;
					continue;
				}


			} else {  // Contains : = label
				std::string label = line.substr(0, colonPosition);
				if (label.empty()) {
					std::stringstream errstream;
					errstream << "ERROR, line " << lineno << " : Empty label name";
					throw std::runtime_error(errstream.str());
				}

				if (label.at(0) == '.')  // Local label
					label = lastLabel + label;
				else  // Global label
					lastLabel = label;

				labels[label] = position;
			}
		}

		std::vector<uint8_t> assembled;

		if (errors > 0)
			return assembled;

		// Resolve labels and assemble
		for (std::vector<asm_instruction_t>::iterator it = instructions.begin(); it != instructions.end(); it++) {
			asm_instruction_t instruction = *it;
			try {
				assembled.push_back(instruction.opcode);
				if (instruction.operands != 0) {
					if (instruction.labelOperand != "") {
						if (labels.find(instruction.labelOperand) == labels.end())
							throw std::runtime_error("Undefined label " + instruction.labelOperand);

						int position = labels.at(instruction.labelOperand);
						if (instruction.labelRelative && instruction.operands == 1) {
							assembled.push_back(uint8_t(position - (assembled.size() + instruction.operands)));
						} else if (instruction.operands == 2) {
							assembled.push_back(uint16_t(position) & 0xFF);
							assembled.push_back(uint16_t(position) >> 8);
						} else {
							throw std::logic_error("BAD OPERAND COUNT FOR LABEL OPERAND");
						}
					} else {
						assembled.push_back(instruction.operand1);
						if (instruction.operands == 2) {
							assembled.push_back(instruction.operand2);
						}
					}
				}
			} catch (std::exception const& exc) {
				errors += 1;
				std::cerr << "ERROR, line " << lineno << " : " << exc.what() << std::endl;
				continue;
			}
		}

		return assembled;
	}
}
