#include "memory/mapping/AudioToneSweepMapping.hpp"

#define OFFSET_START IO_CH1_SWEEP
#define OFFSET_SWEEP    IO_CH1_SWEEP - OFFSET_START
#define OFFSET_PATTERN  IO_CH1_PATTERN - OFFSET_START
#define OFFSET_ENVELOPE IO_CH1_ENVELOPE - OFFSET_START
#define OFFSET_FREQLOW  IO_CH1_FREQLOW - OFFSET_START
#define OFFSET_CONTROL  IO_CH1_CONTROL - OFFSET_START

namespace toygb {
	AudioToneSweepMapping::AudioToneSweepMapping(int channel, AudioControlMapping* control, OperationMode mode) : AudioChannelMapping(channel, control, mode) {
		sweepTime = 0;
		sweepDirection = false;
		sweepShift = 0;

		wavePatternDuty = 2;
		length = 0x3F;

		initialEnvelopeVolume = 0x0F;
		envelopeDirection = false;
		envelopeSweep = 3;

		frequency = 0x07FF;
		stopSelect = false;

		m_dutyPointer = 0;
		m_baseTimerCounter = 0;
		m_lengthTimerCounter = 0;
		m_outputTimerCounter = 0;
		m_envelopeTimerCounter = 0;
		m_sweepTimerCounter = 0;
		m_sweepFrequency = 0;
	}

	uint8_t AudioToneSweepMapping::get(uint16_t address) {
		switch (address) {
			case OFFSET_SWEEP:
				return (0x80 |
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
		if (powered | (m_mode == OperationMode::DMG && address == OFFSET_PATTERN)){
			switch (address) {
				case OFFSET_SWEEP:
					sweepTime = (value >> 4) & 7;
					sweepDirection = (value >> 3) & 1;
					sweepShift = value & 7;
					break;
				case OFFSET_PATTERN:
					if (powered)
						wavePatternDuty = (value >> 6) & 3;
					length = 64 - (value & 0x3F);
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
					stopSelect = (value >> 6) & 1;
					frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);
					if ((value >> 7) & 1)
						reset();
					break;
			}
		}
	}

	const uint8_t TONE_WAVEPATTERNS[4] = {0b00000001, 0b10000001, 0b10000111, 0b01111110};

	void AudioToneSweepMapping::update(){
		m_lengthTimerCounter += 1;
		if (m_lengthTimerCounter >= LENGTH_TIMER_PERIOD){
			m_lengthTimerCounter = 0;
			length -= 1;
			if (stopSelect && length == 0)
				disable();
		}

		if (sweepTime != 0){
			m_sweepTimerCounter += 1;
			if (m_sweepTimerCounter >= sweepTime*SWEEP_TIMER_PERIOD){
				m_sweepTimerCounter = 0;
				updateFrequencySweep();
			}
		}

		m_baseTimerCounter += 1;
		if (m_baseTimerCounter >= 4*(2048 - m_sweepFrequency)){
			m_dutyPointer = (m_dutyPointer + 1) % 8;
			m_baseTimerCounter = 0;
		}

		if (envelopeSweep != 0){
			m_envelopeTimerCounter += 1;
			if (m_envelopeTimerCounter >= envelopeSweep*ENVELOPE_TIMER_PERIOD){
				m_envelopeTimerCounter = 0;

				if (envelopeDirection && m_envelopeVolume < 15)
					m_envelopeVolume += 1;
				else if (m_envelopeVolume > 0)
					m_envelopeVolume -= 1;
			}
		}

		m_outputTimerCounter += 1;
		if (m_outputTimerCounter >= OUTPUT_SAMPLE_PERIOD && m_outputBufferIndex < OUTPUT_BUFFER_SAMPLES){
			m_outputTimerCounter = 0;
			outputSample();
		}
	}

	float AudioToneSweepMapping::buildSample(){
		bool patternValue = (TONE_WAVEPATTERNS[wavePatternDuty] >> (7 - m_dutyPointer)) & 1;
		return (patternValue ? 1.0f : -1.0f) * m_envelopeVolume / 15;
	}

	void AudioToneSweepMapping::updateFrequencySweep(){
		if (sweepShift != 0){
			uint16_t newFrequency = m_sweepFrequency;
			if (sweepDirection)
				newFrequency -= (m_sweepFrequency >> sweepShift);
			else
				newFrequency += (m_sweepFrequency >> sweepShift);

			// Overflow check
			if (newFrequency >= 2048)
				disable();

			frequency = newFrequency;
			m_sweepFrequency = newFrequency;

			if (sweepDirection)
				newFrequency -= (newFrequency >> sweepShift);
			else
				newFrequency += (newFrequency >> sweepShift);

			// Second overflow check
			if (newFrequency >= 2048)
				disable();
		}
	}

	void AudioToneSweepMapping::reset(){
		start();
		m_outputTimerCounter = 0;
		m_baseTimerCounter = 0;
		m_envelopeVolume = initialEnvelopeVolume;
		m_sweepFrequency = frequency;

		updateFrequencySweep();
	}

	void AudioToneSweepMapping::onPowerOff(){
		set(OFFSET_SWEEP, 0);
		set(OFFSET_PATTERN, 0);
		set(OFFSET_ENVELOPE, 0);
		set(OFFSET_FREQLOW, 0);
		set(OFFSET_CONTROL, 0);
	}

	void AudioToneSweepMapping::onPowerOn(){
		m_dutyPointer = 0;
		m_lengthTimerCounter = 0;
		m_envelopeTimerCounter = 0;
		m_sweepTimerCounter = 0;
	}
}
