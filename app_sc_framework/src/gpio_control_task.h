// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include <stdint.h>
#include "i2c.h"
#include "uart.h"
#include "xua_conf.h"
#include "app_dsp.h"

typedef struct control_input_t
{
    uint32_t vu[NUM_USB_CHAN_OUT];
    int32_t output_gain[NUM_USB_CHAN_OUT];
}control_input_t;

#ifdef __XC__
typedef interface adsp_control_if {
    // void dummy(void);
  app_dsp_output_control_t do_control(app_dsp_input_control_t input);
} adsp_control_if;



void gpio_control_task( client uart_tx_if i_uart_tx,
                        chanend c_adc,
                        client interface adsp_control_if i_adsp_control,
                        out buffered port:32 p_neopixel, clock cb_neo,
                        client input_gpio_if i_gpio_mc_buttons[],
                        client output_gpio_if i_gpio_mc_leds[]
                        );

void gpio_control_slave(server interface adsp_control_if i_adsp_control);
#endif