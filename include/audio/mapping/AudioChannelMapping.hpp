// MIT License
//
// Copyright (c) 2021 Tyulis
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef _AUDIO_MAPPING_AUDIOCHANNELMAPPING_HPP
#define _AUDIO_MAPPING_AUDIOCHANNELMAPPING_HPP

// Reference for almost everything in the audio controller : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware

#include "audio/timing.hpp"
#include "audio/mapping/AudioControlMapping.hpp"
#include "audio/mapping/AudioDebugMapping.hpp"
#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** Base class for all audio channels
	 * The channel logic and operation is all implemented within its memory mapping
	 * TODO : Some things could be factored (like length counter, envelope, ...) */
	class AudioChannelMapping : public MemoryMapping {
		public:
			/** Initialize the channel
			 * int channel : channel index (0 = tone+sweep, 1 = tone, 2 = wave, 3 = debug) */
			AudioChannelMapping(int channel, AudioControlMapping* control, AudioDebugMapping* debug, HardwareConfig* hardware);

			/** Unfolds an APU cycle (=2 clocks) worth of channel operation */
			void update();

			/** Return the audio samples that have been generated, or nullptr if not enough samples have been generated to fill the buffer
			 * As such, if this returns a valid buffer, it is always full. Sample values are in range [-1, 1]
			 * The buffer size is specified by audio/timing.hpp:OUTPUT_BUFFER_SAMPLES */
			float* getBuffer();

			void powerOn();   // Called when the APU is powered on (= when NR52.7 goes 0 -> 1)
			void powerOff();  // Called when the APU is powered off (= when NR52.7 goes 1 -> 0)

			bool powered;  // Power status (true = on, false = off)

		protected:
			// Methods to be overridden by subclasses, non-abstract ones have the default implementation for channels that do not have their features
			virtual void onPowerOn();       // Called on power-on
			virtual void onPowerOff();      // Called on power-off
			virtual void onUpdate() = 0;    // Called every APU cycle
			virtual void onSweepFrame();    // Called when the frame sequencer clocks on a sweep frame
			virtual void onLengthFrame();   // Called when the frame sequencer clocks on a length frame
			virtual void onEnvelopeFrame(); // Called when the frame sequencer clocks on an envelope frame

			// Output a sample to be played from the current channel’s state. Value must be in range [-1, 1]
			virtual float buildSample() = 0;

			// Base channel functionality, may be extended to do things at the same time but must not be fully overridden
			virtual void start();         // Start channel output (usually when setting NRx4.7)
			virtual void disable();       // Stop channel output (usually when the length counter falls to 0, sweep overflows, ...)
			virtual void outputSample();  // Output a sample from the current channel’s state (via buildSample()) and put it in the back buffer

			int m_channel;  // Channel index
			AudioControlMapping* m_control;
			AudioDebugMapping* m_debug;
			HardwareConfig* m_hardware;

			bool m_started;  // True if the channel is operating

			// The whole thing is double-buffered
			float* m_outputBuffer;
			float* m_backBuffer;
			int m_outputBufferIndex;  // Next index to set in the back buffer
			bool m_bufferAvailable;   // Whether a buffer is full and available to use

			int m_frameSequencerTimer;  // Counts the APU cycles for the 512Hz frame sequencer
			int m_frameSequencer;       // Current sequencer frame (0-7)
			int m_outputTimerCounter;   // Counts the APU cycles for the output sample frequency

		private:
			void onFrame(int frame);  // Called every time the frame sequencer clocks, dispatches to the individual frame methods
	};
}

#endif
