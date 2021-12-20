#ifndef _MEMORY_MAPPING_AUDIOTONESWEEPMAPPING_HPP
#define _MEMORY_MAPPING_AUDIOTONESWEEPMAPPING_HPP

#include "audio/timing.hpp"
#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/mapping/AudioChannelMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioToneSweepMapping : public AudioChannelMapping {
		public:
			AudioToneSweepMapping(int channel, AudioControlMapping* control, HardwareConfig& hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			uint8_t sweepTime;
			bool sweepDirection;
			uint8_t sweepShift;

			uint8_t wavePatternDuty;
			uint8_t length;

			uint8_t initialEnvelopeVolume;
			bool envelopeDirection;
			uint8_t envelopeSweep;

			uint16_t frequency;
			bool stopSelect;

		protected:
			void reset();
			float buildSample();
			uint16_t calculateFrequencySweep();
			void onPowerOn();
			void onPowerOff();
			void onUpdate();
			void onLengthFrame();
			void onSweepFrame();
			void onEnvelopeFrame();

			int m_envelopeVolume;
			uint16_t m_sweepFrequency;
			int m_dutyPointer;

			int m_baseTimerCounter;
			int m_envelopeFrameCounter;
			int m_sweepFrameCounter;
			bool m_sweepEnabled;
			bool m_sweepNegateCalculated;
	};
}

#endif
