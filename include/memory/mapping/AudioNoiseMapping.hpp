#ifndef _MEMORY_MAPPING_AUDIONOISEMAPPING_HPP
#define _MEMORY_MAPPING_AUDIONOISEMAPPING_HPP

#include "audio/timing.hpp"
#include "core/OperationMode.hpp"
#include "memory/Constants.hpp"
#include "memory/mapping/AudioChannelMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioNoiseMapping : public AudioChannelMapping {
		public:
			AudioNoiseMapping(int channel, AudioControlMapping* control, OperationMode mode);

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

			void update();

		protected:
			void reset();
			float buildSample();
			void onPowerOn();
			void onPowerOff();

			uint16_t m_register;
			int m_envelopeVolume;

			int m_lengthTimerCounter;
			int m_outputTimerCounter;
			int m_envelopeTimerCounter;
			int m_baseTimerCounter;
	};
}

#endif
