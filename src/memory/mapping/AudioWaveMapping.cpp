#include "memory/mapping/AudioWaveMapping.hpp"

#define OFFSET_START IO_CH3_ENABLE
#define OFFSET_ENABLE  IO_CH3_ENABLE - OFFSET_START
#define OFFSET_LENGTH  IO_CH3_LENGTH - OFFSET_START
#define OFFSET_LEVEL   IO_CH3_LEVEL - OFFSET_START
#define OFFSET_FREQLOW IO_CH3_FREQLOW - OFFSET_START
#define OFFSET_CONTROL IO_CH3_CONTROL - OFFSET_START

namespace toygb {
	AudioWaveMapping::AudioWaveMapping(int channel, AudioControlMapping* control) {
		m_channel = channel;
		m_control = control;

		enable = false;
		length = 0xFF;
		outputLevel = 0;
		frequency = 0x07FF;
		stopSelect = false;

		started = false;
		dotCounter = 0;
	}

	uint8_t AudioWaveMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_ENABLE: return (enable << 7) | 0x7F;
			case OFFSET_LENGTH: return length;
			case OFFSET_LEVEL: return (outputLevel << 5) | 0x9F;
			case OFFSET_FREQLOW: return 0xFF;
			case OFFSET_CONTROL:
				return (stopSelect << 6) | 0xBF;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	void AudioWaveMapping::set(uint16_t address, uint8_t value){
		switch (address) {
			case OFFSET_ENABLE: enable = (value >> 7) & 1; break;
			case OFFSET_LENGTH: length = value; break;
			case OFFSET_LEVEL: outputLevel = (value >> 5) & 3; break;
			case OFFSET_FREQLOW:
				frequency = (frequency & 0x0700) | value;
				break;
			case OFFSET_CONTROL:
				started = (value >> 7) & 1;
				dotCounter = 0;
				stopSelect = (value >> 6) & 1;
				frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);
				break;
		}
	}

	void AudioWaveMapping::update(){
		dotCounter += 1;
	}

	int16_t* AudioWaveMapping::getBuffer(){
		return nullptr;
	}
}
