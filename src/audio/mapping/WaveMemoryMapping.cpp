#include "audio/mapping/WaveMemoryMapping.hpp"


#define WAVE_NOT_READABLE 0xFFFF

namespace toygb {
	// Initalize the memory mapping
	WaveMemoryMapping::WaveMemoryMapping(uint8_t* array, HardwareConfig* hardware) : ArrayMemoryMapping(array) {
		m_hardware = hardware;
		m_playing = false;
		m_readIndex = WAVE_NOT_READABLE;
		m_readCounter = 0;
	}

	// Get the value at the given relative address
	uint8_t WaveMemoryMapping::get(uint16_t address) {
		if (m_playing) {
			// While the wave channel is playing, only the sample that is being read can be accessed
			// On DMG hardware, it is only readble at the exact same time the APU is reading
			if (m_readIndex == WAVE_NOT_READABLE)
				return 0xFF;
			else
				return ArrayMemoryMapping::get(m_readIndex);
		} else {  // Normal access when the wave channel is inactive
			return ArrayMemoryMapping::get(address);
		}
	}

	// Set the value at the given relative address
	void WaveMemoryMapping::set(uint16_t address, uint8_t value) {
		if (m_playing) {
			// Only the sample that is being read by the APU is accessible while the wave channel is playing, see ::get
			if (m_readIndex != WAVE_NOT_READABLE)
				return ArrayMemoryMapping::set(m_readIndex, value);
		} else {  // Normal access otherwise
			return ArrayMemoryMapping::set(address, value);
		}
	}

	// Tell that the given index is being read
	void WaveMemoryMapping::setCurrentIndex(uint16_t address){
		m_readIndex = address;
		m_readCounter = 2;
	}

	// Unchecked access for APU operation
	uint8_t WaveMemoryMapping::waveGet(uint16_t address){
		return ArrayMemoryMapping::get(address);
	}

	// Unchecked access for APU operation
	void WaveMemoryMapping::waveSet(uint16_t address, uint8_t value){
		ArrayMemoryMapping::set(address, value);
	}

	// Called every APU cycle (= 2 cycles) while the wave channel is active
	void WaveMemoryMapping::update() {
		// Tick the timer for the currently read index access timer
		if (m_readIndex != WAVE_NOT_READABLE) {
			m_readCounter -= 1;
			if (m_readCounter <= 0){
				m_readIndex = WAVE_NOT_READABLE;
			}
		}
	}

	// Set whether the wave channel is active
	void WaveMemoryMapping::setPlaying(bool playing){
		m_playing = playing;
	}
}
