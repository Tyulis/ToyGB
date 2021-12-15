#include "memory/mapping/AudioWaveMapping.hpp"

#define OFFSET_START IO_CH3_ENABLE
#define OFFSET_ENABLE  IO_CH3_ENABLE - OFFSET_START
#define OFFSET_LENGTH  IO_CH3_LENGTH - OFFSET_START
#define OFFSET_LEVEL   IO_CH3_LEVEL - OFFSET_START
#define OFFSET_FREQLOW IO_CH3_FREQLOW - OFFSET_START
#define OFFSET_CONTROL IO_CH3_CONTROL - OFFSET_START

namespace toygb {
	AudioWaveMapping::AudioWaveMapping(int channel, AudioControlMapping* control, ArrayMemoryMapping* wavePatternMapping, OperationMode mode) : AudioChannelMapping(channel, control, mode) {
		m_wavePatternMapping = wavePatternMapping;

		enable = false;
		length = 0xFF;
		outputLevel = 0;
		frequency = 0x07FF;
		stopSelect = false;

		m_baseTimerCounter = 0;
		m_sampleIndex = 0;
	}

	uint8_t AudioWaveMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_ENABLE: return (enable << 7) | 0x7F;
			case OFFSET_LENGTH: return 0xFF;
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
		if (powered || (m_mode == OperationMode::DMG && address == OFFSET_LENGTH)){
			switch (address) {
				case OFFSET_ENABLE:
					enable = (value >> 7) & 1;
					if (!enable)
						disable();
					break;
				case OFFSET_LENGTH: length = (256 - value); break;
				case OFFSET_LEVEL: outputLevel = (value >> 5) & 3; break;
				case OFFSET_FREQLOW:
					frequency = (frequency & 0x0700) | value;
					break;
				case OFFSET_CONTROL: {
					// If enabling length in first half of the length period
					bool wasEnabled = stopSelect;
					stopSelect = (value >> 6) & 1;
					if (!wasEnabled && stopSelect && m_frameSequencer % 2 == 0 && length > 0)
						onLengthFrame();

					frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);
					if ((value >> 7) & 1)
						reset();
					break;
				}
			}
		}
	}

	void AudioWaveMapping::onUpdate(){
		m_baseTimerCounter += 1;
		if (m_baseTimerCounter >= 2*(2048 - frequency)){
			m_sampleIndex = (m_sampleIndex + 1) % 32;
			m_baseTimerCounter = 0;
		}
	}

	void AudioWaveMapping::onLengthFrame(){
		if (stopSelect){
			length -= 1;
			if (length == 0)
				disable();
		}
	}

	const float WAVE_VOLUMES[] = {0.0f, 1.0f, 0.5f, 0.25f};

	float AudioWaveMapping::buildSample(){
		if (outputLevel > 0 && enable){
			uint8_t sample = (m_wavePatternMapping->get(m_sampleIndex >> 1) >> ((m_sampleIndex & 1) ? 0 : 4)) & 0x0F;
			return (2*sample / 16.0f - 1) * WAVE_VOLUMES[outputLevel];
		} else {
			return 0.0f;
		}
	}

	void AudioWaveMapping::reset(){
		// Only start if DAC is enabled
		if (enable)
			start();

		m_sampleIndex = 1;

		if (length == 0){
			if (m_frameSequencer % 2 == 0 && stopSelect)
				length = 255;
			else
				length = 256;
		}
	}

	void AudioWaveMapping::onPowerOff(){
		set(OFFSET_ENABLE, 0);
		set(OFFSET_LENGTH, 0);
		set(OFFSET_LEVEL, 0);
		set(OFFSET_FREQLOW, 0);
		set(OFFSET_CONTROL, 0);
	}

	void AudioWaveMapping::onPowerOn(){
		m_sampleIndex = 1;
		m_baseTimerCounter = 0;
	}
}
