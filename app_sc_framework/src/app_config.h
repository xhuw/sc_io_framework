// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

// This file contains general app configuration. For USB config, please see xua_conf.h

#pragma once

#define NUM_ADC_POTS        2
#define POT_NUM_BITS        10
#define ADC_LUT_SIZE        (1 << POT_NUM_BITS)
#define ADC_FILTER_DEPTH    32 
#define ADC_HYSTERESIS      3

#define VU_NUM_PIXELS       12

#define UART_BAUD_RATE  115200


