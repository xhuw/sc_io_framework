// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include <stdint.h>
#include "i2c.h"
#include "uart.h"
#include "xua_conf.h"
#include "app_dsp.h"

#warning THIS SHOULD BE BROUGHT IN FROM ADSP.H
#define SIG_QBITS 27
#define UNITY_VOLUME (1 << SIG_QBITS)


int32_t control_to_volume_setting(unsigned vol_setting);

unsigned envelope_to_vu(int32_t envelope);

#ifdef __XC__
typedef interface adsp_control_if {
    // void dummy(void);
  app_dsp_output_control_t do_control(app_dsp_input_control_t input);
} adsp_control_if;



void gpio_control_task( client uart_tx_if i_uart_tx,
                        chanend c_adc,
                        client interface adsp_control_if i_adsp_control,
                        out buffered port:32 p_neopixel, clock cb_neo,
                        client interface i2c_master_if i_i2c,
                        client input_gpio_if i_gpio_mc_buttons[NUM_BUTTONS],
                        client output_gpio_if i_gpio_mc_leds[NUM_LEDS]
                        );

void gpio_control_slave(server interface adsp_control_if i_adsp_control);
#endif