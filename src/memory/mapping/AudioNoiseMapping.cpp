#include "memory/mapping/AudioNoiseMapping.hpp"

#define OFFSET_START IO_CH4_LENGTH
#define OFFSET_LENGTH   IO_CH4_LENGTH - OFFSET_START
#define OFFSET_ENVELOPE IO_CH4_ENVELOPE - OFFSET_START
#define OFFSET_COUNTER  IO_CH4_COUNTER - OFFSET_START
#define OFFSET_CONTROL  IO_CH4_CONTROL - OFFSET_START

namespace toygb {
	AudioNoiseMapping::AudioNoiseMapping(int channel, AudioControlMapping* control) {
		m_channel = channel;
		m_control = control;

		length = 0x3F;

		initialEnvelopeVolume = 0;
		envelopeDirection = false;
		envelopeSweep = 0;

		frequency = 0;
		counterStep = false;
		dividingRatio = 0;
		stopSelect = false;

		started = false;
		dotCounter = 0;
	}

	uint8_t AudioNoiseMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_LENGTH: return 0xFF;
			case OFFSET_ENVELOPE:
				return (
					(initialEnvelopeVolume << 4) |
					(envelopeDirection << 3) |
					envelopeSweep);
			case OFFSET_COUNTER:
				return (
					(frequency << 4) |
					(counterStep << 3) |
					dividingRatio);
			case OFFSET_CONTROL:
				return (stopSelect << 6) | 0xBF;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	void AudioNoiseMapping::set(uint16_t address, uint8_t value){
		switch (address) {
			case OFFSET_LENGTH: length = value & 0x3F; break;
			case OFFSET_ENVELOPE:
				initialEnvelopeVolume = (value >> 4) & 0x0F;
				envelopeDirection = (value >> 3) & 1;
				envelopeSweep = value & 7;
				break;
			case OFFSET_COUNTER:
				frequency = (value >> 4) & 0x0F;
				counterStep = (value >> 3) & 1;
				dividingRatio = value & 7;
				break;
			case OFFSET_CONTROL:
				if ((value >> 7) & 1){
					started = true;
					dotCounter = 0;
				}
				stopSelect = (value >> 6) & 1;
				break;
		}
	}

	void AudioNoiseMapping::update() {
		dotCounter += 1;
	}

	int16_t* AudioNoiseMapping::getBuffer(){
		return nullptr;
	}
}
