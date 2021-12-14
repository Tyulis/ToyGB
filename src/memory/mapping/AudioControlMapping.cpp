#include "memory/mapping/AudioControlMapping.hpp"

#define OFFSET_START IO_AUDIO_LEVELS
#define OFFSET_LEVELS IO_AUDIO_LEVELS - OFFSET_START
#define OFFSET_OUTPUT IO_AUDIO_OUTPUT - OFFSET_START
#define OFFSET_ENABLE IO_AUDIO_ENABLE - OFFSET_START

namespace toygb {
	AudioControlMapping::AudioControlMapping() {
		vinOutput1 = false;
		vinOutput2 = false;
		output1Level = 7;
		output2Level = 7;

		output1Channels[0] = true;
		output1Channels[1] = true;
		output1Channels[2] = false;
		output1Channels[3] = false;
		output2Channels[0] = true;
		output2Channels[1] = true;
		output2Channels[2] = true;
		output2Channels[3] = true;

		audioEnable = true;
		channelEnable[0] = false;
		channelEnable[1] = false;
		channelEnable[2] = false;
		channelEnable[3] = false;
	}

	uint8_t AudioControlMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_LEVELS:
				return (
					(vinOutput2 << 7) |
					(output2Level << 4) |
					(vinOutput1 << 3) |
					output1Level);
			case OFFSET_OUTPUT: {
				uint8_t result = 0x00;
				for (int i = 0; i < 4; i++)
					result |= output2Channels[i] << (i + 4);
				for (int i = 0; i < 4; i++)
					result |= output1Channels[i] << i;
				return result;
			}
			case OFFSET_ENABLE: {
				uint8_t result = audioEnable << 7;
				for (int i = 0; i < 4; i++)
					result |= channelEnable[i] << i;
				return result | 0x70;
			}
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	void AudioControlMapping::set(uint16_t address, uint8_t value){
		if (audioEnable || address == OFFSET_ENABLE){
			switch (address) {
				case OFFSET_LEVELS:
					vinOutput2 = (value >> 7) & 1;
					output2Level = (value >> 4) & 7;
					vinOutput1 = (value >> 3) & 1;
					output1Level = value & 7;
					break;
				case OFFSET_OUTPUT:
					for (int i = 0; i < 4; i++)
						output2Channels[i] = (value >> (i + 4)) & 1;
					for (int i = 0; i < 4; i++)
						output1Channels[i] = (value >> i) & 1;
					break;
				case OFFSET_ENABLE:
					bool newEnable = (value >> 7) & 1;
					if (audioEnable && !newEnable)
						onPowerOff();
					else if (!audioEnable && newEnable)
						onPowerOn();
					audioEnable = newEnable;
					break;
			}
		}
	}

	void AudioControlMapping::onPowerOff(){
		set(OFFSET_LEVELS, 0);
		set(OFFSET_OUTPUT, 0);
	}

	void AudioControlMapping::onPowerOn(){

	}
}
