#ifndef _AUDIO_MAPPING_AUDIODEBUGMAPPING_HPP
#define _AUDIO_MAPPING_AUDIODEBUGMAPPING_HPP

#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Implements the undocumented CGB registers in range FF72-FF77, some of which are probably for audio controller debugging (PCM12, PCM34)
	* The purpose of the others is unknown (if any), but well, they are contiguous so might as well implement them here too */
	class AudioDebugMapping : public MemoryMapping {
		public:
			AudioDebugMapping(HardwareConfig& hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			// Set the current 4-bits PCM sample for a channel
			void setChannelAmplitude(int channel, uint8_t value);

		private:
			HardwareConfig m_hardware;

			uint8_t m_amplitudes[4];  // Current PCM sample for each channel (index is the channel index)

			// Undocumented registers content
			uint8_t m_ff72;
			uint8_t m_ff73;
			uint8_t m_ff74;
			uint8_t m_ff75;
	};
}

#endif
