#ifndef _MEMORY_MAPPING_AUDIOCHANNELMAPPING_HPP
#define _MEMORY_MAPPING_AUDIOCHANNELMAPPING_HPP

#include "audio/timing.hpp"
#include "core/OperationMode.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "memory/mapping/AudioControlMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	class AudioChannelMapping : public MemoryMapping {
		public:
			AudioChannelMapping(int channel, AudioControlMapping* control, OperationMode mode);

			void update();
			float* getBuffer();

			void powerOn();
			void powerOff();

			bool powered;

		protected:
			virtual void onPowerOn();
			virtual void onPowerOff();
			virtual void onUpdate() = 0;
			virtual void onSweepFrame();
			virtual void onLengthFrame();
			virtual void onEnvelopeFrame();

			void start();
			void disable();
			void outputSample();
			virtual float buildSample() = 0;

			int m_channel;
			AudioControlMapping* m_control;
			OperationMode m_mode;

			bool m_started;

			float* m_outputBuffer;
			float* m_backBuffer;
			int m_outputBufferIndex;
			bool m_bufferAvailable;

			int m_frameSequencerTimer;
			int m_frameSequencer;
			int m_outputTimerCounter;


		private:
			void onFrame(int frame);
	};
}

#endif
