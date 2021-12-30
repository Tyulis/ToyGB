#ifndef _DEBUG_ASSEMBLER_HPP
#define _DEBUG_ASSEMBLER_HPP

#include <map>
#include <string>
#include <vector>
#include <cctype>
#include <cstring>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>


namespace toygb {
	/** Utilities to assemble and disassemble Gameboy assembler code on-the-fly */

	// Assemble the given code into an array of bytes
	std::vector<uint8_t> assemble(std::string code);

	// Disassemble some machine code into assembler code
	std::string disassemble(uint8_t* code, int size);
}

#endif
