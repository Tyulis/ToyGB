#include "memory/mapping/AudioToneMapping.hpp"

#define OFFSET_START IO_CH2_PATTERN
#define OFFSET_PATTERN  IO_CH2_PATTERN - OFFSET_START
#define OFFSET_ENVELOPE IO_CH2_ENVELOPE - OFFSET_START
#define OFFSET_FREQLOW  IO_CH2_FREQLOW - OFFSET_START
#define OFFSET_CONTROL  IO_CH2_CONTROL - OFFSET_START

namespace toygb {
	AudioToneMapping::AudioToneMapping() {
		wavePatternDuty = 0;
		length = 0x3F;

		initialEnvelopeVolume = 0;
		envelopeDirection = false;
		envelopeSweep = 0;

		frequency = 0x07FF;
		initialize = false;
		stopSelect = false;
	}

	uint8_t AudioToneMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_PATTERN: return (wavePatternDuty << 6) | 0x3F;
			case OFFSET_ENVELOPE:
				return (
					(initialEnvelopeVolume << 4) |
					(envelopeDirection << 3) |
					envelopeSweep);
			case OFFSET_FREQLOW: return 0xFF;
			case OFFSET_CONTROL:
				return (stopSelect << 6) | 0xBF;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	void AudioToneMapping::set(uint16_t address, uint8_t value){
		switch (address) {
			case OFFSET_PATTERN:
				wavePatternDuty = (value >> 6) & 3;
				length = value & 0x3F;
				break;
			case OFFSET_ENVELOPE:
				initialEnvelopeVolume = (value >> 4) & 0x0F;
				envelopeDirection = (value >> 3) & 1;
				envelopeSweep = value & 7;
				break;
			case OFFSET_FREQLOW:
				frequency = (frequency & 0x0700) | value;
				break;
			case OFFSET_CONTROL:
				initialize = (value >> 7) & 1;
				stopSelect = (value >> 6) & 1;
				frequency = (frequency & 0x00FF) | (value & 0x07);
				break;
		}
	}
}
