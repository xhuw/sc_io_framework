// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.


#pragma once
#include <xccompat.h>
#include <stdint.h>


// control inputs, from control thread to DSP
typedef struct {
    int32_t game_loopback_switch_pos;
    // mic 1/2/headset vol
    int32_t mic_vol;
    uint8_t mic_mute;

    // USB/BT/live vol
    int32_t music_vol;
    uint8_t music_mute;

    // headphone/headset vol
    int32_t monitor_vol;
    int8_t monitor_mute;

    // usb/live out vol
    int32_t output_vol;
    int8_t output_mute;

    // reverb
    float reverb_wet_gain; // reverb level, value between -186 and 0
    int32_t reverb_enable;

    // denoise/ducking toggles
    int32_t denoise_enable;
    int32_t ducking_enable;
} app_dsp_input_control_t;

#define _MAX_XCORE_THREADS 8

// control outputs, from DSP to control thread.
typedef struct {
    int32_t mic_envelope;
    int32_t headphone_envelope;
    uint32_t max_thread_ticks[_MAX_XCORE_THREADS];
} app_dsp_output_control_t;

void app_dsp_do_control(REFERENCE_PARAM(app_dsp_input_control_t, input), REFERENCE_PARAM(app_dsp_output_control_t, output));

void app_dsp_init(void);

// send data to dsp
void app_dsp_source(REFERENCE_PARAM(int32_t, data));

// read output
void app_dsp_sink(REFERENCE_PARAM(int32_t, data));

// do dsp
void app_dsp_main(chanend c_control);


void app_dsp_main_local_control(void);

