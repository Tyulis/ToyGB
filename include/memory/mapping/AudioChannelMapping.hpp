#ifndef _MEMORY_MAPPING_AUDIOCHANNELMAPPING_HPP
#define _MEMORY_MAPPING_AUDIOCHANNELMAPPING_HPP

#include "audio/timing.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioChannelMapping : public MemoryMapping {
		public:
			AudioChannelMapping(int channel, AudioControlMapping* control);

			virtual void update() = 0;
			int16_t* getBuffer();

			void disable();

		protected:
			void start();
			void outputSample();
			virtual int16_t buildSample() = 0;

			int m_channel;
			AudioControlMapping* m_control;

			bool m_started;

			int16_t* m_outputBuffer;
			int16_t* m_backBuffer;
			int m_outputBufferIndex;
			bool m_bufferAvailable;
	};
}

#endif
