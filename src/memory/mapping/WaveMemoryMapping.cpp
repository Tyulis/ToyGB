#include "memory/mapping/WaveMemoryMapping.hpp"
#include <iostream>
#include "util/error.hpp"

#define WAVE_NOT_READABLE 0xFFFF

namespace toygb {
	WaveMemoryMapping::WaveMemoryMapping(uint8_t* array, HardwareConfig& hardware) : ArrayMemoryMapping(array) {
		m_hardware = hardware;
		m_playing = false;
		m_readIndex = WAVE_NOT_READABLE;
		m_readCounter = 0;
	}

	uint8_t WaveMemoryMapping::get(uint16_t address){
		if (m_playing){
			if (m_readIndex == WAVE_NOT_READABLE)
				return 0xFF;
			else
				return ArrayMemoryMapping::get(m_readIndex);
		} else {
			return ArrayMemoryMapping::get(address);
		}
	}

	void WaveMemoryMapping::set(uint16_t address, uint8_t value){
		if (m_playing){
			if (m_readIndex != WAVE_NOT_READABLE)
				return ArrayMemoryMapping::set(m_readIndex, value);
		} else {
			return ArrayMemoryMapping::set(address, value);
		}
	}

	void WaveMemoryMapping::setCurrentIndex(uint16_t address){
		m_readIndex = address;
		m_readCounter = 2;
	}

	uint8_t WaveMemoryMapping::waveGet(uint16_t address){
		return ArrayMemoryMapping::get(address);
	}

	void WaveMemoryMapping::waveSet(uint16_t address, uint8_t value){
		ArrayMemoryMapping::set(address, value);
	}

	void WaveMemoryMapping::update(){
		if (m_readIndex != WAVE_NOT_READABLE){
			m_readCounter -= 1;
			if (m_readCounter <= 0){
				m_readIndex = WAVE_NOT_READABLE;
			}
		}
	}

	void WaveMemoryMapping::setPlaying(bool playing){
		m_playing = playing;
	}
}
