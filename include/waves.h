#ifndef _WAVES
#define _WAVES

//  TODO: Envelopes
//  TODO: Free parameters, input macros

/*
   (c) Tatu Laras 2025

   waves - A library for modular FM sound synthesis.

   See README.md for usage instructions and examples, along with the function
   descriptions below.

   Depends on my "vec.h" dynamic list macro helper.
   Make sure to also include it in your include folder along with this file.
*/

#include "vec.h"
#include <stdint.h>

// This is the handle for the "output" waveform, which will make the wave
// audible.
#define WAVES_OUTPUT 1

typedef size_t WaveformHandle;
VEC_DECLARE(WaveformHandle, WaveformHandleVec, wfhandlevec)

typedef enum {
    WAVES_WAVEFORM_SINE,
    WAVES_WAVEFORM_TRIANGLE,
    WAVES_WAVEFORM_SAW,
    WAVES_WAVEFORM_SQUARE,
    WAVES_WAVEFORM_OUTPUT,
} WaveformType;

typedef struct {
    float attack;
    float decay;
    float sustain;
    float release;
} Envelope;

typedef struct {
    float output_amplitude;
    float modulation_amplitude;
    float frequency_ratio;
    WaveformType type;
    WaveformHandleVec inputs;
    Envelope envelope;
} Waveform;

typedef struct {
    float press_time;
    float release_time;
    float frequency;
    uint8_t velocity;
    uint8_t active;
} Note;

VEC_DECLARE(Waveform, WaveformVec, wfvec)

// Initializes the library, call this before any other function.
void waves_init(float sample_rate);

// Create a new waveform. Will not affect anything unless connected with
// waves_connect_waveforms().
// `output_amplitude` and `modulation_amplitude` will be the wave's amplitude if
// connected to the audible output waveform or another waveform for modulation
// (a carrier), respectively.
WaveformHandle waves_new_waveform(WaveformType type, float frequency_ratio,
                                  float output_amplitude,
                                  float modulation_amplitude);

void waves_waveform_set_envelope(WaveformHandle handle, Envelope envelope);

// Connects waveform `from` to modulate waveform `to`.
// Modulation amount depends on the modulation_amplitude of the waveform `from`.
// A waveform can be made audible by connecting it to the output waveform, the
// handle of which is the constant WAVES_OUTPUT.
void waves_connect_waveforms(WaveformHandle from, WaveformHandle to);

void waves_note_on(uint8_t note, uint8_t velocity);
void waves_note_off(uint8_t note);
void waves_all_notes_off(void);

// Synthezises a single float32 mono pcm frame, using the waveform configuration
// created using waves_new_waveform() and waves_connect_waveforms().
float waves_get_frame(void);

#endif
