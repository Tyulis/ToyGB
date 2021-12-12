#ifndef _MEMORY_MAPPING_AUDIOTONESWEEPMAPPING_HPP
#define _MEMORY_MAPPING_AUDIOTONESWEEPMAPPING_HPP

#include "audio/timing.hpp"
#include "memory/Constants.hpp"
#include "memory/mapping/AudioChannelMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioToneSweepMapping : public AudioChannelMapping {
		public:
			AudioToneSweepMapping(int channel, AudioControlMapping* control);

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

			void update();

		private:
			void reset();
			float buildSample();
			void updateFrequencySweep();

			int m_envelopeVolume;
			int m_sweepFrequency;

			int m_lengthTimerCounter;
			int m_baseTimerCounter;
			int m_outputTimerCounter;
			int m_envelopeTimerCounter;
			int m_sweepTimerCounter;

			uint64_t m_baseTimer;
	};
}

#endif
