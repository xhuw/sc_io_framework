// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <print.h>
#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "gpio_control_task.h"
#include "dsp.h" // lib_dsp


int32_t control_to_volume_setting(unsigned vol_setting){
    q8_24 lin_volume = vol_setting << (24 - POT_NUM_BITS);
    q8_24 pow_volume = dsp_math_exp(lin_volume) - dsp_math_exp(0);
    // printf("lin: %ld pow: %ld unity: %d\n", lin_volume, pow_volume, UNITY_VOLUME);

    // mul by 4.77 which scales from max setting to unity volume
    int32_t vol = (4775 * (int64_t)pow_volume) >> 10;

    // printf("vol: %ld unity: %d\n", vol, UNITY_VOLUME);
    return vol;
}


unsigned envelope_to_vu(int32_t envelope){
    uq8_24 envelope_uq = envelope;
    q8_24 log_vu = dsp_math_log(envelope_uq + (1 << 24));

    unsigned level = log_vu / (10000000 / VU_NUM_PIXELS);
    printf("envelope: %ld log_vu: %ld level: %u\n", envelope, log_vu, level);

    return level;
}
