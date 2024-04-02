// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "app_dsp.h"
#include "stdbool.h"
#include "xcore/chanend.h"
#include "xcore/parallel.h"
#include "dspt_control.h"
#include "xua_conf.h"

#include <stages/adsp_pipeline.h>
#include <stdint.h>
#include "adsp_generated_auto.h"

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
