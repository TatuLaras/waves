#include "miniaudio.h"
#include "waves.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define DEVICE_FORMAT ma_format_f32
#define DEVICE_CHANNELS 2
#define DEVICE_SAMPLE_RATE 48000

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                   ma_uint32 frameCount) {
    float *frames_out = (float *)pOutput;
    for (uint32_t i_frame = 0; i_frame < frameCount; i_frame += 1) {

        // The only interaction with waves in this function
        float value = waves_get_frame();

        for (uint32_t i_channel = 0; i_channel < pDevice->playback.channels;
             i_channel += 1) {
            frames_out[i_frame * pDevice->playback.channels + i_channel] =
                value;
        }
    }

    (void)pInput;
}

int main(int argc, char **argv) {
    waves_init(DEVICE_SAMPLE_RATE);

    WaveformHandle carrier =
        waves_new_waveform(WAVES_WAVEFORM_SINE, 1.0, 0.2, 0);

    WaveformHandle m1 = waves_new_waveform(WAVES_WAVEFORM_SINE, 1.5, 0, 0.2);
    WaveformHandle m2 = waves_new_waveform(WAVES_WAVEFORM_SINE, 0.5, 0, 6.0);

    waves_connect_waveforms(m1, carrier);
    waves_connect_waveforms(m2, carrier);
    waves_connect_waveforms(carrier, WAVES_OUTPUT);

    waves_waveform_set_envelope(carrier, (Envelope){
                                             .attack = 0.1,
                                             .sustain = 0.6,
                                             .decay = 0.6,
                                             .release = 1.0,
                                         });
    waves_waveform_set_envelope(m1, (Envelope){
                                        .attack = 0.1,
                                        .sustain = 0.4,
                                        .decay = 0.3,
                                        .release = 0.4,
                                    });
    waves_waveform_set_envelope(m2, (Envelope){
                                        .attack = 0.1,
                                        .sustain = 0.2,
                                        .decay = 0.5,
                                        .release = 0.4,
                                    });

    ma_device_config deviceConfig;
    ma_device device;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = DEVICE_FORMAT;
    deviceConfig.playback.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback = data_callback;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return -4;
    }

    printf("Device Name: %s\n", device.playback.name);

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return -5;
    }

    printf("q+enter to quit\n");
    printf("enter to toggle notes on and off\n");
    while (1) {
        waves_note_on(55, 1);
        waves_note_on(62, 1);
        if (getchar() == 'q')
            break;
        waves_note_off(55);
        waves_note_off(62);
        if (getchar() == 'q')
            break;
    }

    ma_device_uninit(&device);

    (void)argc;
    (void)argv;
    return 0;
}
