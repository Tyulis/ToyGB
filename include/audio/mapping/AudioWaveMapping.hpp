#ifndef _AUDIO_MAPPING_AUDIOWAVEMAPPING_HPP
#define _AUDIO_MAPPING_AUDIOWAVEMAPPING_HPP

#include "audio/timing.hpp"
#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "audio/mapping/AudioChannelMapping.hpp"
#include "audio/mapping/AudioControlMapping.hpp"
#include "audio/mapping/WaveMemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Custom wave channel memory mapping and operation */
	class AudioWaveMapping : public AudioChannelMapping {
		public:
			AudioWaveMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, WaveMemoryMapping* wavePatternMapping, HardwareStatus* hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			// NR30 : Channel enable flag
			bool enable;  // Enable the channel's DAC (register NR30, bit 7)

			// NR31 : Sound length counter
			uint16_t length;  // Sound length counter, uint16 because it may be 256 (register NR31, bits 0-7)

			// NR32 : Volume control
			uint8_t outputLevel;  // Output volume level (0-3) (register NR32, bits 5-6)

			// NR33 + NR34.0-3 : Sound frequency control
			uint16_t frequency;  // The channel state is updated every 2*(2048 - `frequency`)

			// NR34 : Channel control
			bool enableLength;  // Enables length counter operation (1 = enable, channel stops when length counter reaches zero, 0 = disable) (register NR34, bit 6)

		protected:
			void reset();  // Called when a channel restart is requested via NR14.7

			// AudioChannelMapping overrides
			float buildSample();
			void onPowerOn();
			void onPowerOff();
			void onUpdate();
			void onLengthFrame();
			void disable();
			void start();

			WaveMemoryMapping* m_wavePatternMapping;  // Wave RAM

			int m_baseTimerCounter;  // Counts cycles for the recalculation period
			int m_sampleIndex;       // Current sample index in wave RAM
	};
}

#endif
