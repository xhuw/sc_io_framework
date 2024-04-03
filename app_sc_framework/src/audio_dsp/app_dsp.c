// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "app_dsp.h"
#include "stdbool.h"
#include "xcore/chanend.h"
#include "xcore/parallel.h"
#include "xcore/assert.h"
#include "dspt_control.h"
#include "xua_conf.h"

#include <stages/adsp_pipeline.h>
#include <stages/adsp_control.h>
#include <stdint.h>
#include "adsp_generated_auto.h"
#include "adsp_instance_id_auto.h"
#include "cmds.h"

// static adsp_pipeline_t * m_dsp;
adsp_pipeline_t * m_dsp;

static void arr_to_pointer(int32_t** pointer, int32_t* arr, int n) {
    for(int i = 0; i < n; ++i) {
        pointer[i] = &arr[i];
    }
}

// send data to dsp
void app_dsp_source(REFERENCE_PARAM(int32_t, data)) {
    int32_t* in_data[NUM_USB_CHAN_OUT + I2S_CHANS_DAC];
    arr_to_pointer(in_data, data, NUM_USB_CHAN_OUT + I2S_CHANS_DAC);
    adsp_pipeline_source(m_dsp, in_data);
}

// read output
void app_dsp_sink(REFERENCE_PARAM(int32_t, data)) {
    int32_t* out_data[NUM_USB_CHAN_IN + I2S_CHANS_ADC];
    arr_to_pointer(out_data, data, NUM_USB_CHAN_IN + I2S_CHANS_ADC);
    adsp_pipeline_sink(m_dsp, out_data);
}


// do dsp
void app_dsp_main_local_control(void) {
    m_dsp = adsp_auto_pipeline_init();
    adsp_auto_pipeline_main(m_dsp);
}

static void do_write(int instance, int cmd_id, int size, void* data) {
    adsp_stage_control_cmd_t cmd = {
        .instance_id = instance,
        .cmd_id = cmd_id,
        .payload_len = size,
        .payload = data
    };
    for(;;) {
        adsp_control_status_t ret = adsp_write_module_config(
                m_dsp->modules,
                m_dsp->n_modules,
                &cmd);
        xassert(ADSP_CONTROL_ERROR != ret);
        if(ADSP_CONTROL_SUCCESS == ret) {
            return;
        }
    }
}
static void do_read(int instance, int cmd_id, int size, void* data) {
    adsp_stage_control_cmd_t cmd = {
        .instance_id = instance,
        .cmd_id = cmd_id,
        .payload_len = size,
        .payload = data
    };
    for(;;) {
        adsp_control_status_t ret = adsp_write_module_config(
                m_dsp->modules,
                m_dsp->n_modules,
                &cmd);
        xassert(ADSP_CONTROL_ERROR != ret);
        if(ADSP_CONTROL_SUCCESS == ret) {
            return;
        }
    }
}

void app_dsp_do_control(REFERENCE_PARAM(app_dsp_input_control_t, input), REFERENCE_PARAM(app_dsp_output_control_t, output)) {
    // vol
    do_write(mic_vc_stage_index, CMD_VOLUME_CONTROL_TARGET_GAIN, sizeof(int32_t), &input->mic_vol);
    do_write(music_vc_stage_index, CMD_VOLUME_CONTROL_TARGET_GAIN, sizeof(int32_t), &input->music_vol);
    do_write(monitor_vc_stage_index, CMD_VOLUME_CONTROL_TARGET_GAIN, sizeof(int32_t), &input->monitor_vol);
    do_write(output_vc_stage_index, CMD_VOLUME_CONTROL_TARGET_GAIN, sizeof(int32_t), &input->monitor_vol);
    // mute
    do_write(mic_vc_stage_index, CMD_VOLUME_CONTROL_TARGET_GAIN, sizeof(int8_t), &input->mic_mute);
    do_write(music_vc_stage_index, CMD_VOLUME_CONTROL_TARGET_GAIN, sizeof(int8_t), &input->music_mute);
    do_write(monitor_vc_stage_index, CMD_VOLUME_CONTROL_TARGET_GAIN, sizeof(int8_t), &input->monitor_mute);
    do_write(output_vc_stage_index, CMD_VOLUME_CONTROL_TARGET_GAIN, sizeof(int8_t), &input->output_mute);

    do_write(game_loopback_switch_ch0_stage_index, CMD_SWITCH_POSITION, sizeof(int32_t), &input->game_loopback_switch_pos);
    do_write(game_loopback_switch_ch1_stage_index, CMD_SWITCH_POSITION, sizeof(int32_t), &input->game_loopback_switch_pos);
    do_write(denoise_enable_stage_index, CMD_SWITCH_POSITION, sizeof(int32_t), &input->denoise_enable);
    do_write(reverb_enable_stage_index, CMD_SWITCH_POSITION, sizeof(int32_t), &input->reverb_enable);
    do_write(duck0_enable_stage_index, CMD_SWITCH_POSITION, sizeof(int32_t), &input->ducking_enable);
    do_write(duck1_enable_stage_index, CMD_SWITCH_POSITION, sizeof(int32_t), &input->ducking_enable);

    do_read(mic_vu_stage_index, CMD_ENVELOPE_DETECTOR_RMS_ENVELOPE, sizeof(int32_t), &output->mic_envelope);
    do_read(out_vu_stage_index, CMD_ENVELOPE_DETECTOR_RMS_ENVELOPE, sizeof(int32_t), &output->headphone_envelope);
}
