#include "memory/mapping/InterruptRegisterMapping.hpp"


namespace toygb {
	InterruptRegisterMapping::InterruptRegisterMapping(){
		for (int i = 0; i < 5; i++){
			interrupts[i] = false;
		}
	}

	uint8_t InterruptRegisterMapping::get(uint16_t address){
		uint8_t result = 0x00;
		for (int i = 0; i < 5; i++){
			result |= interrupts[i] << i;
		}
		return result;
	}

	void InterruptRegisterMapping::set(uint16_t address, uint8_t value){
		for (int i = 0; i < 5; i++){
			interrupts[i] = (value >> i) & 1;
		}
	}
}
