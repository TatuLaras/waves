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
        waves_new_waveform(WAVES_WAVEFORM_SINE, 220, 0.2, 0);

    WaveformHandle m1 =
        waves_new_waveform(WAVES_WAVEFORM_SINE, 220.0 / 4, 0, 1.0);
    WaveformHandle m2 = waves_new_waveform(WAVES_WAVEFORM_SINE, 220.0, 0, 0.2);

    waves_connect_waveforms(m1, carrier);
    waves_connect_waveforms(m2, m1);
    waves_connect_waveforms(carrier, WAVES_OUTPUT);

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

    printf("Press Enter to quit...\n");
    getchar();

    ma_device_uninit(&device);

    (void)argc;
    (void)argv;
    return 0;
}
