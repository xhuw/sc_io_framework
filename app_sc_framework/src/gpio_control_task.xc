// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <print.h>
#include <stdio.h>
#include <string.h>
#include <platform.h>

#include "app_config.h"
#include "gpio_control_task.h"
#include "neopixel.h"
#include "adc_pot.h"
#include "app_dsp.h"
#include "dsp/adsp.h"

#define VU_GREEN    0x000010
#define VU_RED      0x002000
#define VU_OFF      0x000000

#define UNITY_VOLUME (1 << Q_SIG)

unsafe void vu_to_pixels(control_input_t * unsafe control_input, neopixel_state &np_state){
    for(int i = 0; i < 12; i++){
        int32_t threshold = 1 << (2 * i + 7);
        if(control_input->vu[0] > threshold){
            if(i < 8){
                np_state.data[i] = VU_GREEN;
            } else {
                np_state.data[i] = VU_RED;
            }
        } else {
            np_state.data[i] = VU_OFF;
        }

        if(control_input->vu[1] > threshold){
            if(i < 8){
                np_state.data[23 - i] = VU_GREEN;
            } else {
                np_state.data[23 - i] = VU_RED;
            }
        } else {
            np_state.data[23 - i] = VU_OFF;
        }
    }
}


void gpio_control_task( client uart_tx_if i_uart_tx,
                        chanend c_adc, control_input_t * unsafe control_input,
                        out buffered port:32 p_neopixel, clock cb_neo,
                        client input_gpio_if i_gpio_mc_buttons,
                        client output_gpio_if i_gpio_mc_leds
                        ){
    printf("gpio_control_task\n");

    // Neopixel setup
    neopixel_state np_state = {0};
    const unsigned length = 24;
    neopixel_init(np_state, length, p_neopixel, cb_neo, 3);

    // UART data
    const uint8_t msg[] = "Hello world!\n";
    unsigned msg_idx = 0;

    // DSP control
    app_dsp_input_control_t dsp_input = {0};
    dsp_input.game_loopback_switch_pos = 1;
    dsp_input.mic_vol = UNITY_VOLUME;
    dsp_input.mic_mute = 0;
    dsp_input.music_vol = UNITY_VOLUME;
    dsp_input.music_mute = 0;
    dsp_input.monitor_vol = UNITY_VOLUME;
    dsp_input.monitor_mute = 0;
    dsp_input.output_vol = UNITY_VOLUME;
    dsp_input.output_mute = 0;
    dsp_input.reverb_level = 0;
    dsp_input.reverb_enable = 1;
    dsp_input.denoise_enable = 1;
    dsp_input.ducking_enable = 1;

    app_dsp_output_control_t dsp_output = {0};

    // Main control super loop
    while(1)unsafe{
        // Drive VU on neopixels
        vu_to_pixels(control_input, np_state);
        while(!neopixel_drive_pins(np_state, p_neopixel)); // Takes about 1.2 ms for 24 neopixels

        // Read ADCs for pot input
        unsigned adc[NUM_ADC_POTS] = {0};
        unsigned adc_dir[NUM_ADC_POTS] = {0};
        printf("ADC ");
        for(unsigned ch = 0; ch < NUM_ADC_POTS; ch++){
            c_adc <: (uint32_t)(ADC_CMD_READ | ch);
            c_adc :> adc[ch];
            c_adc <: (uint32_t)(ADC_CMD_POT_GET_DIR | ch);
            c_adc :> adc_dir[ch];
            printf("ch %u: %u (%u) ", ch, adc[ch], adc_dir[ch]);
            control_input->output_gain[ch] = (int64_t)adc[ch] * (int64_t)INT_MAX / (ADC_LUT_SIZE - 1);
        }
        printf("\n");
        app_dsp_do_control(dsp_input, dsp_output);
        printf("Envelope %6d %6d\n", dsp_output.mic_envelope, dsp_output.headphone_envelope);

        // Read buttons
        unsigned pb = i_gpio_mc_buttons.input();
        if((pb & 0x1) == 0){ // Button 0 pressed
            dsp_input.output_vol = UNITY_VOLUME / 2;
        } else {
            dsp_input.output_vol = UNITY_VOLUME;

        }

        // Drive MC leds
        i_gpio_mc_leds.output(pb);


        // Send a character to the UART
        i_uart_tx.write(msg[msg_idx]);
        if(++msg_idx == strlen((const char*)msg)){
            msg_idx = 0;
        }

        delay_milliseconds(10);
    }
}

