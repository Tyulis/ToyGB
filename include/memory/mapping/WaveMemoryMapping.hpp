#ifndef _MEMORY_MAPPING_WAVEMEMORYMAPPING_HPP
#define _MEMORY_MAPPING_WAVEMEMORYMAPPING_HPP

#include "core/OperationMode.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"

namespace toygb {
	class WaveMemoryMapping : public ArrayMemoryMapping {
		public:
			WaveMemoryMapping(uint8_t* array, OperationMode mode);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);
			uint8_t waveGet(uint16_t address);
			void waveSet(uint16_t address, uint8_t value);

			void update();
			void setPlaying(bool playing);
			void setCurrentIndex(uint16_t index);

		protected:
			OperationMode m_mode;
			uint16_t m_readIndex;
			int m_readCounter;
			bool m_playing;
	};
}

#endif
