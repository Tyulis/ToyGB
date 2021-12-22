#ifndef _AUDIO_TIMING_HPP
#define _AUDIO_TIMING_HPP

#include "core/timing.hpp"

// APU updates every 2 clocks (0x200000 Hz)
#define APU_CLOCK_FREQUENCY (CLOCK_FREQUENCY / 2)

// Arbitrary output sample rate, better if it is a divisor of the APU clock frequency
#define OUTPUT_SAMPLE_FREQUENCY 49152

// Frame sequencer low-frequency clock, 512Hz
#define FRAME_SEQUENCER_FREQUENCY 512

// Output and frame sequencer periods in APU cycles (= period in clocks / 2)
#define OUTPUT_SAMPLE_PERIOD (APU_CLOCK_FREQUENCY / OUTPUT_SAMPLE_FREQUENCY)
#define FRAME_SEQUENCER_PERIOD (APU_CLOCK_FREQUENCY / FRAME_SEQUENCER_FREQUENCY)

// Arbitrary amount of samples per buffer, empirically 1024 is not too bad with SFML
#define OUTPUT_BUFFER_SAMPLES 1024

#endif
