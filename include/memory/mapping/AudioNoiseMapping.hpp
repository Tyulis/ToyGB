#ifndef _MEMORY_MAPPING_AUDIONOISEMAPPING_HPP
#define _MEMORY_MAPPING_AUDIONOISEMAPPING_HPP

#include "audio/timing.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioNoiseMapping : public MemoryMapping {
		public:
			AudioNoiseMapping(int channel, AudioControlMapping* control);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			uint8_t length;

			uint8_t initialEnvelopeVolume;
			bool envelopeDirection;
			uint8_t envelopeSweep;

			uint8_t frequency;
			bool counterStep;
			uint8_t dividingRatio;

			bool stopSelect;

			bool started;
			uint64_t dotCounter;

			void update();
			int16_t* getBuffer();

		private:
			int m_channel;
			AudioControlMapping* m_control;
	};
}

#endif
