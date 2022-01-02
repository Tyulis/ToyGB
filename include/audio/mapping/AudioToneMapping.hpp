#ifndef _AUDIO_MAPPING_AUDIOTONEMAPPING_HPP
#define _AUDIO_MAPPING_AUDIOTONEMAPPING_HPP

#include "audio/timing.hpp"
#include "audio/mapping/AudioChannelMapping.hpp"
#include "audio/mapping/AudioControlMapping.hpp"
#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Tone (square wave) channel memory mapping and operation (channel 2) */
	class AudioToneMapping : public AudioChannelMapping {
		public:
			AudioToneMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, HardwareStatus* hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			// NR21 : Length and pattern duty
			uint8_t wavePatternDuty;  // Select the wave pattern duty (0-3) (register NR21, bits 6-7)
			uint8_t length;           // Sound length counter (0-64) (register NR21, bits 0-5)

			// NR22 : Envelope control
			uint8_t initialEnvelopeVolume;  // Initial envelope volume value (0-15) (register NR22, bits 4-7)
			bool envelopeDirection;         // Envelope direction (0 = decrease volume, 1 = increase)
			uint8_t envelopePeriod;         // Envelope period (envelope volume is recalculated every `envelopePeriod` frames, 0 disables envelope) (register NR22, bits 0-2)

			// NR43 + NR44.0-2 : Base clock frequency
			uint16_t frequency;  // The channel state is updated every 2*(2048 - `frequency`)

			// NR44 : Channel control
			bool enableLength;  // Enables length counter operation (1 = enable, channel stops when length counter reaches zero, 0 = disable) (register NR24, bit 6)

		protected:
			void reset();  // Called when a channel restart is requested via NR24.7

			// AudioChannelMapping overrides
			float buildSample();
			void onPowerOn();
			void onPowerOff();
			void onUpdate();
			void onLengthFrame();
			void onEnvelopeFrame();

			int m_envelopeVolume;  // Current envelope volume
			int m_dutyPointer;     // Current position in the wave pattern duty

			int m_baseTimerCounter;      // Counts cycles for the recalculation period
			int m_envelopeFrameCounter;  // Counts envelope frames for the envelope update period
	};
}

#endif
