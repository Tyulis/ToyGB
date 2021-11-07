#ifndef _MEMORY_MAPPING_AUDIONOISEMAPPING_HPP
#define _MEMORY_MAPPING_AUDIONOISEMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioNoiseMapping : public MemoryMapping {
		public:
			AudioNoiseMapping();

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			uint8_t length;

			uint8_t initialEnvelopeVolume;
			bool envelopeDirection;
			uint8_t envelopeSweep;

			uint8_t frequency;
			bool counterStep;
			uint8_t dividingRatio;

			bool initialize;
			bool stopSelect;
	};
}

#endif
