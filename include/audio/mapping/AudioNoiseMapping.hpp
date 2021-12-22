#ifndef _AUDIO_MAPPING_AUDIONOISEMAPPING_HPP
#define _AUDIO_MAPPING_AUDIONOISEMAPPING_HPP

#include "audio/timing.hpp"
#include "audio/mapping/AudioChannelMapping.hpp"
#include "audio/mapping/AudioControlMapping.hpp"
#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Noise channel implementation (channel 4) */
	class AudioNoiseMapping : public AudioChannelMapping {
		public:
			AudioNoiseMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, HardwareConfig& hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			// NR41 : Sound length
			uint8_t length;  // Length counter current value (ticked down as the frame sequencer clocks down) (set by NR41, bits 0-5)

			// NR42 : Volume envelope control
			uint8_t initialEnvelopeVolume;  // Initial envelope value (0-15) (register NR42, bits 4-7)
			bool envelopeDirection;         // Envelope direction (0 = decrease volume, 1 = increase) (register NR42, bit 3)
			uint8_t envelopePeriod;         // Envelope period (envelope volume is recalculated every `envelopePeriod` frames, 0 disables envelope) (register NR42, bits 0-2)

			// NR43 : Noise generation control
			uint8_t periodShift;  // Amount of bits to shift the recalculation period by (register NR43, bits 4-7)
			bool registerWidth;   // LFSR useful size (0 = 15 bits, 1 = 7 bits) (register NR43, bit 3)
			uint8_t periodBase;   // Sets the base value of the recalculation period (lookup table in AudioNoiseMapping.cpp) (register NR43, bits 0-2)

			// NR44 : Channel control
			bool enableLength;  // Enables length counter operation (1 = enable, channel stops when length counter reaches zero, 0 = disable) (register NR44, bit 6)

		protected:
			void reset();  // Called when a channel restart is requested via NR44.7

			// AudioChannelMapping overrides
			float buildSample();
			void onPowerOn();
			void onPowerOff();
			void onUpdate();
			void onLengthFrame();
			void onEnvelopeFrame();

			uint16_t m_register;   // Holds the current LFSR (linear feedback shift register) value (15 bits)
			int m_envelopeVolume;  // Current envelope volume

			int m_baseTimerCounter;      // Counts cycles for the recalculation period
			int m_envelopeFrameCounter;  // Counts envelope frames for the envelope update period
	};
}

#endif
