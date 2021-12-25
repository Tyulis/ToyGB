#ifndef _AUDIO_MAPPING_AUDIOTONESWEEPMAPPING_HPP
#define _AUDIO_MAPPING_AUDIOTONESWEEPMAPPING_HPP

#include "audio/timing.hpp"
#include "audio/mapping/AudioChannelMapping.hpp"
#include "audio/mapping/AudioControlMapping.hpp"
#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Tone (square wave) channel with frequency sweep memory mapping and operation (channel 2) */
	class AudioToneSweepMapping : public AudioChannelMapping {
		public:
			AudioToneSweepMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, HardwareConfig& hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			// NR10 : Frequency sweep control
			uint8_t sweepPeriod;  // Number of sweep frames between every sweep update (0-7) (register NR10, bits 4-6)
			bool sweepDirection;  // Frequency sweep direction (0 = increase frequency, 1 = decrease) (register NR10, bit 3)
			uint8_t sweepShift;   // Frequency sweep shift (0-7) (register NR10, bits 0-2)

			// NR11 : Sound pattern and length control
			uint8_t wavePatternDuty;  // Select the wave pattern duty (0-3) (register NR11, bits 6-7)
			uint8_t length;           // Sound length counter (0-64) (register NR11, bits 0-5)

			// NR12 : Volume envelope control
			uint8_t initialEnvelopeVolume;  // Initial envelope volume value (0-15) (register NR12, bits 4-7)
			bool envelopeDirection;         // Envelope direction (0 = decrease volume, 1 = increase)
			uint8_t envelopePeriod;         // Envelope period (envelope volume is recalculated every `envelopePeriod` frames, 0 disables envelope) (register NR12, bits 0-2)

			// NR13 + NR14.0-2 : Sound frequency control
			uint16_t frequency;  // The channel state is updated every 2*(2048 - `frequency`)

			// NR14 : Channel control
			bool enableLength;  // Enables length counter operation (1 = enable, channel stops when length counter reaches zero, 0 = disable) (register NR14, bit 6)

		protected:
			void reset();  // Called when a channel restart is requested via NR14.7
			uint16_t calculateFrequencySweep();  // Perform a sweep frequency calculation and overflow check

			// AudioChannelMapping overrides
			float buildSample();
			void onPowerOn();
			void onPowerOff();
			void onUpdate();
			void onLengthFrame();
			void onSweepFrame();
			void onEnvelopeFrame();

			int m_envelopeVolume;       // Current volume envelope value
			uint16_t m_sweepFrequency;  // Current sound frequency, with sweep
			int m_dutyPointer;          // Current position in the wave duty

			int m_baseTimerCounter;        // Counts cycles for the recalculation period
			int m_envelopeFrameCounter;    // Counts envelope frames for the envelope update period
			int m_sweepFrameCounter;       // Counts sweep frames for the sweep update period
			bool m_sweepEnabled;           // Internal sweep enable frag
			bool m_sweepNegateCalculated;  // Whether a sweep calculation in negate (decreasing frequency) mode has already been calculated since the last channel restart
	};
}

#endif
