#include "audio/mapping/AudioWaveMapping.hpp"

#define OFFSET_START IO_CH3_ENABLE
#define OFFSET_ENABLE  IO_CH3_ENABLE - OFFSET_START
#define OFFSET_LENGTH  IO_CH3_LENGTH - OFFSET_START
#define OFFSET_LEVEL   IO_CH3_LEVEL - OFFSET_START
#define OFFSET_FREQLOW IO_CH3_FREQLOW - OFFSET_START
#define OFFSET_CONTROL IO_CH3_CONTROL - OFFSET_START

/** Wave channel control IO registers mapping

(Access is R for read-only, W for write-only, B for both, - for none)
Abs. addr. | Rel. addr. | Name | Access   | Content
      FF1A |       0000 | NR30 | B------- | Channel DAC enable flag (bit 7)
      FF1B |       0000 | NR31 | WWWWWWWW | Set the sound length counter (0-255)
      FF1C |       0001 | NR32 | -BB----- | Output volume level (0 = mutes, 1 = 100%, 2 = 50%, 3 = 25%) (bits 5-6)
      FF1D |       0002 | NR33 | WWWWWWWW | Lower 8 bits of the frequency control
      FF1E |       0003 | NR34 | WB---WWW | Channel enable control : SL---FFF
           |            |      |          | - S (bit 7) : Trigger the channel operation (set to 1 to start)
           |            |      |          | - L (bit 6) : Enable the length counter (0 = disabled, 1 = counting, stops the channel when it reaches zero)
           |            |      |          | - F (bits 0-2) : Higher 3 bits of the frequency control */


namespace toygb {
	// The output amplitude is multiplied by WAVE_VOLUMES[outputLevel]
	const float WAVE_VOLUMES[] = {0.0f, 1.0f, 0.5f, 0.25f};
	
	// Initialize the channel
	// Most of the initial values are just to have the right values at boot, they will get reset as soon as the channel is used
	AudioWaveMapping::AudioWaveMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, WaveMemoryMapping* wavePatternMapping, HardwareConfig& hardware) : AudioChannelMapping(channel, control, debug, hardware) {
		m_wavePatternMapping = wavePatternMapping;

		enable = false;
		length = 0xFF;
		outputLevel = 0;
		frequency = 0x07FF;
		enableLength = false;

		m_baseTimerCounter = 0;
		m_sampleIndex = 0;
	}

	// Get the value at the given relative address
	uint8_t AudioWaveMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_ENABLE:  // NR30
				return (enable << 7) | 0x7F;
			case OFFSET_LENGTH:  // NR31
				return 0xFF;
			case OFFSET_LEVEL:  // NR32
				return (outputLevel << 5) | 0x9F;
			case OFFSET_FREQLOW:  // NR33
				return 0xFF;
			case OFFSET_CONTROL:  // NR34
				return (enableLength << 6) | 0xBF;
		}
		std::stringstream errstream;
		errstream << "Wrong memory mapping : " << oh16(address);
		throw EmulationError(errstream.str());
	}

	// Set the value at the given relative address
	void AudioWaveMapping::set(uint16_t address, uint8_t value) {
		// Ignore writes when the APU is powered off
		// On DMG hardware, length registers are still fully writable even with the APU powered off
		if (powered || (m_hardware.isDMGConsole() && address == OFFSET_LENGTH)) {
			switch (address) {
				case OFFSET_ENABLE:  // NR30
					enable = (value >> 7) & 1;
					if (!enable)
						disable();
					break;
				case OFFSET_LENGTH:  // NR31
					length = (256 - value);
					break;
				case OFFSET_LEVEL:  // NR32
					outputLevel = (value >> 5) & 3;
					break;
				case OFFSET_FREQLOW:  // NR33
					frequency = (frequency & 0x0700) | value;
					break;
				case OFFSET_CONTROL: {  // NR34
					bool wasEnabled = enableLength;
					enableLength = (value >> 6) & 1;
					
					// Enabling length counter in first half of the length period (next frame does not clock length) clocks length once
					if (!wasEnabled && enableLength && m_frameSequencer % 2 == 0 && length > 0)
						onLengthFrame();

					frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);
					if ((value >> 7) & 1)
						reset();
					break;
				}
			}
		}
	}

	// Called at every APU cycle (= 2 clocks)
	void AudioWaveMapping::onUpdate() {
		if (m_started){
			m_baseTimerCounter -= 1;
			if (m_baseTimerCounter <= 0){  // Update period is 2048 - frequency (not 2*(2048-frequency) like the others)
				m_baseTimerCounter = 2048 - frequency;
				m_sampleIndex = (m_sampleIndex + 1) % 32;  // Advance by one, cycling through the 32 samples
				// Wave RAM access during channel operation depends on the sample being accessed. As our sample read and output is not related to the gameboy operation,
				// we need to notify the wave RAM mapping specifically
				m_wavePatternMapping->setCurrentIndex(m_sampleIndex >> 1);
			}
			m_wavePatternMapping->update();  // Tell the wave RAM one APU cycle has passed (for wave RAM access during channel operation shenanigans)
		}
	}

	// Called at every frame that ticks length
	void AudioWaveMapping::onLengthFrame() {
		if (enableLength) {
			length -= 1;
			if (length == 0)
				disable();
		}
	}

	// Output a sample based of the current state of the channel
	float AudioWaveMapping::buildSample() {
		if (outputLevel > 0 && enable) {
			uint8_t sample = (m_wavePatternMapping->waveGet(m_sampleIndex >> 1) >> ((m_sampleIndex & 1) ? 0 : 4)) & 0x0F;
			return (2*sample / 16.0f - 1) * WAVE_VOLUMES[outputLevel];
		} else {
			return 0.0f;
		}
	}

	// Restart the channel operation
	void AudioWaveMapping::reset(){
		// Only start if DAC is enabled
		if (enable)
			start();
			
		// Increment the timer by 3 on trigger to solve some wave RAM access shenanigans
		// https://forums.nesdev.org/viewtopic.php?t=13730&p=188035
		// (Binji gives a delay of 6 clocks, as we operate in APU cycles it's divided by 2)
		m_sampleIndex = 0;
		m_baseTimerCounter = 2048 - frequency + 3;

		// If the length counter is set to zero, it is reloaded with maximum
		if (length == 0){
			// If length is enabled and the next frame does not clock length, maximum - 1
			if (m_frameSequencer % 2 == 0 && enableLength)
				length = 255;
			else
				length = 256;
		}
	}

	// Called when the APU is powered off, set everything to zero
	void AudioWaveMapping::onPowerOff() {
		set(OFFSET_ENABLE, 0);
		set(OFFSET_LENGTH, 0);
		set(OFFSET_LEVEL, 0);
		set(OFFSET_FREQLOW, 0);
		set(OFFSET_CONTROL, 0);
	}

	// Called when the APU is powered on, reset the sample pointer
	void AudioWaveMapping::onPowerOn() {
		m_sampleIndex = 1;
		m_baseTimerCounter = 0;
	}
	
	// Extend AudioChannelMapping::start to manage wave RAM
	void AudioWaveMapping::start() {
		// Trigger while the channel reads a sample : corrupt first bytes of wave RAM
		// Here the APU is emulated after the CPU in the same cycle, and this method is called by the CPU, so we are projecting the next APU cycle operations
		// But for all intents and purposes, trigger is done more or less at the exact same time the APU reads a sample
		if (m_baseTimerCounter - 1 == 0 && m_started){
			uint16_t position = ((m_sampleIndex + 1) >> 1) & 0x0F;  // Index of the sample that would have been read
			if (position < 4) {  // Read in the first 4 bytes : overwrite the first byte with the current one
				m_wavePatternMapping->waveSet(0, m_wavePatternMapping->waveGet(position));
			} else {
				// Overwrite the first four bytes with the current block of four (4-7 / 8-11 / 12-15)
				for (int i = 0; i < 4; i++) {
					m_wavePatternMapping->waveSet(i, m_wavePatternMapping->waveGet((position & 0b1100) + i));
				}
			}
		}

		m_wavePatternMapping->setPlaying(true);
		AudioChannelMapping::start();
	}

	// Extend AudioChannelMapping::disable to manage wave RAM
	void AudioWaveMapping::disable(){
		m_wavePatternMapping->setPlaying(false);
		AudioChannelMapping::disable();
	}
}
