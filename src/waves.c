#include "waves.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

VEC_IMPLEMENT(Waveform, WaveformVec, wfvec)
VEC_IMPLEMENT(WaveformHandle, WaveformHandleVec, wfhandlevec)

static WaveformVec waveforms = {0};
static double waveform_sample_rate = 0;

void waves_init(double sample_rate) {
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
}

WaveformHandle waves_new_waveform(WaveformType type, double frequency,
                                  double output_amplitude,
                                  double modulation_amplitude) {
    assert(waveforms.data);
    assert(waveform_sample_rate);

    return wfvec_append(
        &waveforms,

        (Waveform){
            .output_amplitude = output_amplitude,
            .modulation_amplitude = modulation_amplitude,
            .type = type,
            .advance_amount = (1.0 / (waveform_sample_rate / frequency)),
            .inputs = wfhandlevec_init(),
        });
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

static inline float waveform_get_frame(WaveformHandle handle,
                                       int connected_to_output) {
    assert(waveforms.data);
    assert(handle);

    Waveform *wf = wfvec_get(&waveforms, handle);
    assert(wf);

    float modulation = 0;
    for (size_t i = 0; i < wf->inputs.data_used; i++) {
        WaveformHandle *input_handle = wfhandlevec_get(&wf->inputs, i);
        assert(input_handle);
        modulation += waveform_get_frame(*input_handle,
                                         wf->type == WAVES_WAVEFORM_OUTPUT);
    }

    float value = 0;
    float amplitude =
        connected_to_output ? wf->output_amplitude : wf->modulation_amplitude;

    switch (wf->type) {
    case WAVES_WAVEFORM_OUTPUT:
        value = modulation;
        break;
    case WAVES_WAVEFORM_SINE:
        value = sin((wf->time + modulation) * 2 * M_PI) * amplitude;
        break;

    case WAVES_WAVEFORM_TRIANGLE:
    case WAVES_WAVEFORM_SAW:
    case WAVES_WAVEFORM_SQUARE:
        fprintf(stderr, "ERROR: Trying to play unsupported waveform type %u.\n",
                wf->type);
        abort();
        break;
    }

    wf->time += wf->advance_amount;
    return value;
}

float waves_get_frame(void) {
    return waveform_get_frame(WAVES_OUTPUT, 0);
}
