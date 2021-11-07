#include "memory/mapping/AudioToneSweepMapping.hpp"

#define OFFSET_START IO_CH1_SWEEP
#define OFFSET_SWEEP    IO_CH1_SWEEP - OFFSET_START
#define OFFSET_PATTERN  IO_CH1_PATTERN - OFFSET_START
#define OFFSET_ENVELOPE IO_CH1_ENVELOPE - OFFSET_START
#define OFFSET_FREQLOW  IO_CH1_FREQLOW - OFFSET_START
#define OFFSET_CONTROL  IO_CH1_CONTROL - OFFSET_START

namespace toygb {
	AudioToneSweepMapping::AudioToneSweepMapping() {
		sweepTime = 0;
		sweepDirection = false;
		sweepShift = 0;

		wavePatternDuty = 2;
		length = 0x3F;

		initialEnvelopeVolume = 0x0F;
		envelopeDirection = false;
		envelopeSweep = 3;

		frequency = 0x07FF;
		initialize = false;
		stopSelect = false;
	}

	uint8_t AudioToneSweepMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_SWEEP:
				return (
					(sweepTime << 4) |
					(sweepDirection << 3) |
					sweepShift);
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

	void AudioToneSweepMapping::set(uint16_t address, uint8_t value){
		switch (address) {
			case OFFSET_SWEEP:
				sweepTime = (value >> 4) & 7;
				sweepDirection = (value >> 3) & 1;
				sweepShift = value & 7;
				break;
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
