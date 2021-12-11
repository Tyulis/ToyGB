#ifndef _AUDIO_TIMING_HPP
#define _AUDIO_TIMING_HPP

#include "core/timing.hpp"

#define LENGTH_TIMER_FREQUENCY 256
#define ENVELOPE_TIMER_FREQUENCY 64
#define SWEEP_TIMER_FREQUENCY 128
#define OUTPUT_SAMPLE_FREQUENCY 32768

#define LENGTH_TIMER_PERIOD (CLOCK_FREQUENCY / LENGTH_TIMER_FREQUENCY)
#define ENVELOPE_TIMER_PERIOD (CLOCK_FREQUENCY / ENVELOPE_TIMER_FREQUENCY)
#define SWEEP_TIMER_PERIOD (CLOCK_FREQUENCY / SWEEP_TIMER_FREQUENCY)
#define OUTPUT_SAMPLE_PERIOD (CLOCK_FREQUENCY / OUTPUT_SAMPLE_FREQUENCY)

#define OUTPUT_BUFFER_SAMPLES 512

#endif