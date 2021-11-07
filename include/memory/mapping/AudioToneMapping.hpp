#ifndef _MEMORY_MAPPING_AUDIOTONEMAPPING_HPP
#define _MEMORY_MAPPING_AUDIOTONEMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioToneMapping : public MemoryMapping {
		public:
			AudioToneMapping();

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			uint8_t wavePatternDuty;
			uint8_t length;

			uint8_t initialEnvelopeVolume;
			bool envelopeDirection;
			uint8_t envelopeSweep;

			uint16_t frequency;
			bool initialize;
			bool stopSelect;
	};
}

#endif
