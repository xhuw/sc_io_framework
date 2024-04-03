// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.


#pragma once
#include <xccompat.h>
#include <stdint.h>


// control inputs, from control thread to DSP
typedef struct {
    int32_t game_loopback_switch_pos;
    int32_t mic_vol;
    uint8_t mic_mute;
    int32_t music_vol;
    uint8_t music_mute;
    int32_t monitor_vol;
    int8_t monitor_mute;
    int32_t output_vol;
    int8_t output_mute;
    int32_t reverb_level;
    int32_t reverb_enable;
    int32_t denoise_enable;
    int32_t ducking_enable;
} app_dsp_input_control_t;

// control outputs, from DSP to control thread.
typedef struct {
    int32_t mic_envelope;
    int32_t headphone_envelope;
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

