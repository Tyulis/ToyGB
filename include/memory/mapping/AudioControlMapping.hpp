#ifndef _MEMORY_MAPPING_AUDIOCONTROLMAPPING_HPP
#define _MEMORY_MAPPING_AUDIOCONTROLMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioControlMapping : public MemoryMapping {
		public:
			AudioControlMapping();

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			bool vinOutput2;
			uint8_t output2Level;
			bool vinOutput1;
			uint8_t output1Level;

			bool output2Channels[4];
			bool output1Channels[4];

			bool audioEnable;
			bool channelEnable[4];
	};
}

#endif
