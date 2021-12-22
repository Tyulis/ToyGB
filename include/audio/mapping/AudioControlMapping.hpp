#ifndef _AUDIO_MAPPING_AUDIOCONTROLMAPPING_HPP
#define _AUDIO_MAPPING_AUDIOCONTROLMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** APU control IO registers mapping */
	class AudioControlMapping : public MemoryMapping {
		public:
			AudioControlMapping();

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			// NR50 : Output volume control. Output 1 is right, output 2 is left. Vin is not implemented (nor used in any meaningful cartridge, for all that matters)
			bool vinOutput2;       // Enable Vin output to channel 2 (1 = enabled, 0 = disabled) (register NR50, bit 7)
			uint8_t output2Level;  // Output 2 master volume (0-7) (register NR50, bit 4-6)
			bool vinOutput1;       // Enable Vin output to channel 1 (1 = enabled, 0 = disabled) (register NR50, bit 3)
			uint8_t output1Level;  // Output 1 master volume (0-7) (register NR50, bit 0-2)

			// NR51 : Channels output selection. Output 1 is right, output 2 is left
			bool output2Channels[4];  // Output channels to output 2 (array index is the channel index, 1 = enable, 0 = disable) (register NR51, bits 4-7)
			bool output1Channels[4];  // Output channels to output 1 (1 = enable, 0 = disable) (register NR51, bits 0-3)

			// NR52 : APU global control and channel status
			bool audioEnable;       // APU power switch (1 = power on, 0 = power off) (register NR52, bit 7)
			bool channelEnable[4];  // Individual channels status (1 = operating, a priori as stated by their start() and disable() method, 0 = idle) (register NR52, bits 0-3)

		private:
			void onPowerOn();  // Called when audioEnable goes 0 -> 1
			void onPowerOff(); // Called when audioEnable goes 1 -> 0
	};
}

#endif
