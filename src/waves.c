#include "waves.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

VEC_IMPLEMENT(Waveform, WaveformVec, wfvec)
VEC_IMPLEMENT(WaveformHandle, WaveformHandleVec, wfhandlevec)

static WaveformVec waveforms = {0};
static float waveform_sample_rate = 0;
static float max_release = 0;
static double time = 0;

#define MIDI_NOTE_COUNT 128
static Note notes[MIDI_NOTE_COUNT] = {0};

void waves_init(float sample_rate) {
    assert(sample_rate);

    waveforms = wfvec_init();
    assert(waveforms.data);

    // Handle 0 will be a null handle
    wfvec_append(&waveforms, (Waveform){0});
    // Handle 1 will be the output node
    wfvec_append(&waveforms, (Waveform){
                                 .type = WAVES_WAVEFORM_OUTPUT,
                                 .inputs = wfhandlevec_init(),
                             });

    waveform_sample_rate = sample_rate;

    for (uint8_t i = 0; i < MIDI_NOTE_COUNT; i++) {
        notes[i].frequency = 440.0 * pow(2, (float)(i - 69) / 12);
    }
}

WaveformHandle waves_new_waveform(WaveformType type, float frequency_ratio,
                                  float output_amplitude,
                                  float modulation_amplitude) {
    assert(waveforms.data);
    assert(waveform_sample_rate);

    return wfvec_append(&waveforms,

                        (Waveform){
                            .output_amplitude = output_amplitude,
                            .modulation_amplitude = modulation_amplitude,
                            .type = type,
                            .frequency_ratio = frequency_ratio,
                            .inputs = wfhandlevec_init(),
                            .envelope = {.sustain = 1.0},
                        });
}

void waves_waveform_set_envelope(WaveformHandle handle, Envelope envelope) {
    assert(handle);
    assert(waveforms.data);

    Waveform *wf = wfvec_get(&waveforms, handle);
    assert(wf);

    wf->envelope = envelope;
    max_release = fmax(max_release, envelope.release);
}

void waves_connect_waveforms(WaveformHandle from, WaveformHandle to) {
    assert(from);
    assert(waveforms.data);
    assert(to);

    Waveform *wf = wfvec_get(&waveforms, to);
    assert(wf);
    assert(wf->inputs.data);

    wfhandlevec_append(&wf->inputs, from);
}

static inline float decay(Envelope *envelope, float time_since_press) {
    float decay_amount =
        (time_since_press - envelope->attack) / envelope->decay;
    decay_amount = decay_amount > 1.0 ? 1.0 : decay_amount;
    return 1.0 - (1.0 - envelope->sustain) * decay_amount;
}

static inline float release_multiplier(Envelope *envelope, Note *note) {
    float time_since_press = time - note->press_time;
    float time_since_release = time - note->release_time;

    float current_level = envelope->sustain;
    if (time_since_press < envelope->attack) {
        current_level = time_since_press / envelope->attack;
    } else if (time_since_press < envelope->attack + envelope->decay) {
        current_level = decay(envelope, time_since_press);
    }

    if (time_since_release >= max_release) {
        note->active = 0;
        return 0;
    }

    float release_amount =
        fmax(0.0, 1.0 - time_since_release / envelope->release);
    return current_level * release_amount;
}

static inline float calculate_envelope_multiplier(Envelope *envelope,
                                                  Note *note) {
    if (note->release_time > note->press_time)
        return release_multiplier(envelope, note);

    float time_since_press = time - note->press_time;

    if (time_since_press < envelope->attack)
        return time_since_press / envelope->attack;

    return decay(envelope, time_since_press);
}

static float waveform_get_frame(WaveformHandle handle, Note *note,
                                int use_output_amplitude) {
    assert(waveforms.data);
    assert(handle);

    Waveform *wf = wfvec_get(&waveforms, handle);
    assert(wf);

    float modulation = 0;
    for (size_t i = 0; i < wf->inputs.data_used; i++) {
        WaveformHandle *input_handle = wfhandlevec_get(&wf->inputs, i);
        assert(input_handle);
        modulation += waveform_get_frame(*input_handle, note,
                                         wf->type == WAVES_WAVEFORM_OUTPUT);
    }

    float output_amplitude = 0;
    float wf_amplitude =
        use_output_amplitude ? wf->output_amplitude : wf->modulation_amplitude;

    wf_amplitude *= calculate_envelope_multiplier(&wf->envelope, note);

    switch (wf->type) {
    case WAVES_WAVEFORM_OUTPUT:
        output_amplitude = modulation;
        break;
    case WAVES_WAVEFORM_SINE:
        output_amplitude =
            sin(time * 2 * M_PI * note->frequency * wf->frequency_ratio +
                modulation) *
            wf_amplitude;
        break;

    case WAVES_WAVEFORM_TRIANGLE:
    case WAVES_WAVEFORM_SAW:
    case WAVES_WAVEFORM_SQUARE:
        fprintf(stderr, "ERROR: Trying to play unsupported waveform type %u.\n",
                wf->type);
        abort();
        break;
    }

    return output_amplitude;
}

void waves_note_on(uint8_t note, uint8_t velocity) {
    assert(note < MIDI_NOTE_COUNT);
    notes[note].velocity = velocity;
    notes[note].active = 1;
    notes[note].press_time = time;
}

void waves_note_off(uint8_t note) {
    assert(note < MIDI_NOTE_COUNT);
    notes[note].release_time = time;
}

void waves_all_notes_off(void) {
    for (uint32_t i = 0; i < MIDI_NOTE_COUNT; i++)
        notes[i].release_time = time;
}

float waves_get_frame(void) {
    time += 1.0 / waveform_sample_rate;

    float sample = 0;

    for (uint8_t i = 0; i < MIDI_NOTE_COUNT; i++) {
        if (notes[i].active) {
            printf("note %u\n", i);
            sample += waveform_get_frame(WAVES_OUTPUT, notes + i, 0);
        }
    }
    printf("asd\n");

    return sample;
}
