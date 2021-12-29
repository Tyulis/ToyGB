#include "audio/mapping/AudioControlMapping.hpp"

#define OFFSET_START IO_AUDIO_LEVELS
#define OFFSET_LEVELS IO_AUDIO_LEVELS - OFFSET_START
#define OFFSET_OUTPUT IO_AUDIO_OUTPUT - OFFSET_START
#define OFFSET_ENABLE IO_AUDIO_ENABLE - OFFSET_START

/** APU general control IO registers mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF24 |       0000 | NR50 | BBBBBBBB | Output master volume control : V222W111
           |            |      |          | - V (bit 7) : Output vin channel to output channel 2 (left) (1 = output, 0 = not)
           |            |      |          | - W (bit 3) : Output vin channel to output channel 1 (right) (1 = output, 0 = not)
           |            |      |          | - 2 (bits 4-6) : Output 2 (left) master volume (0-7)
           |            |      |          | - 1 (bits 0-2) : Output 1 (right) master volume (0-7)
      FF25 |       0001 | NR51 | BBBBBBBB | Channels output selection : 22221111
           |            |      |          | - 2 (bits 4-7) : Channels output to output 2 (left), 1 = enable, 0 = not. Channels are in order (bit 4 = channel 1, bit 7 = channel 4)
           |            |      |          | - 1 (bits 0-3) : Channels output to output 1 (right), 1 = enable, 0 = not. Channels are in order (bit 0 = channel 1, bit 3 = channel 4)
      FF26 |       0002 | NR52 | B---RRRR | APU power control and individual channels status : P---CCCC
           |            |      |          | - P (bit 7) : APU power switch (1 = power on, 0 = power off)
           |            |      |          | - C (bits 0-3) : Audio channels status (for each channel, 1 = playing, 0 = idle). Channels are in order (bit 0 = channel 1, bit 3 = channel 4) */

namespace toygb {
	AudioControlMapping::AudioControlMapping(HardwareConfig* hardware) {
		m_hardware = hardware;

		// Default values at boot
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

		audioEnable = !m_hardware->hasBootrom();  // The gameboy apparently starts with the APU disabled, the bootrom enables it later
		channelEnable[0] = false;
		channelEnable[1] = false;
		channelEnable[2] = false;
		channelEnable[3] = false;
	}

	// Get the value at the given relative address
	uint8_t AudioControlMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_LEVELS:  // NR50
				return (
					(vinOutput2 << 7) |
					(output2Level << 4) |
					(vinOutput1 << 3) |
					output1Level);
			case OFFSET_OUTPUT: {  // NR51
				uint8_t result = 0x00;
				for (int i = 0; i < 4; i++)
					result |= output2Channels[i] << (i + 4);
				for (int i = 0; i < 4; i++)
					result |= output1Channels[i] << i;
				return result;
			}
			case OFFSET_ENABLE: {  // NR52
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

	// Set the value at the given relative address
	void AudioControlMapping::set(uint16_t address, uint8_t value) {
		// Ignore writes when the APU is powered off, except to the power control
		if (audioEnable || address == OFFSET_ENABLE) {
			switch (address) {
				case OFFSET_LEVELS:
					vinOutput2 = (value >> 7) & 1;
					output2Level = (value >> 4) & 7;
					vinOutput1 = (value >> 3) & 1;
					output1Level = value & 7;
					break;
				case OFFSET_OUTPUT:
					for (int i = 0; i < 4; i++) {
						output2Channels[i] = (value >> (i + 4)) & 1;
						output1Channels[i] = (value >> i) & 1;
					}
					break;
				case OFFSET_ENABLE:
					bool newEnable = (value >> 7) & 1;
					if (audioEnable && !newEnable)  // 1 -> 0 : power off
						onPowerOff();
					else if (!audioEnable && newEnable)  // 0 -> 1 : power on
						onPowerOn();
					audioEnable = newEnable;
					break;
			}
		}
	}

	// Called when the APU is powered off (everything is set to 0)
	void AudioControlMapping::onPowerOff() {
		set(OFFSET_LEVELS, 0);
		set(OFFSET_OUTPUT, 0);
	}

	// Called when the APU is powered on
	void AudioControlMapping::onPowerOn() {
		// nop
	}
}
