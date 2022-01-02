#ifndef _AUDIO_MAPPING_WAVEMEMORYMAPPING_HPP
#define _AUDIO_MAPPING_WAVEMEMORYMAPPING_HPP

#include "core/hardware.hpp"
#include "memory/mapping/ArrayMemoryMapping.hpp"

namespace toygb {
	/** Wave RAM memory mapping */
	class WaveMemoryMapping : public ArrayMemoryMapping {
		public:
			WaveMemoryMapping(uint8_t* array, HardwareStatus* m_hardware);

			// Access by the CPU
			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			// Access by the APU (unchecked and untransformed)
			uint8_t waveGet(uint16_t address);
			void waveSet(uint16_t address, uint8_t value);

			void update();  // Called at every APU cycle while the wave channel is active

			void setPlaying(bool playing);         // For the APU to tell whether the wave channel is playing
			void setCurrentIndex(uint16_t index);  // Tell that the given index is being read by the APU

		protected:
			HardwareStatus* m_hardware;
			uint16_t m_readIndex;  // Index that is being read
			int m_readCounter;     // Counts the APU cycles to disable access in DMG mode
			bool m_playing;
	};
}

#endif
