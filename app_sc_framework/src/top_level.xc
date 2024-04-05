// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>

#include "app_config.h"
#include "adc_pot.h"
#include "gpio_control_task.h"
#include "xua_conf.h"
#include "i2c.h"
#include "xua.h"
#include "xud.h"
#include "app_dsp.h"


// Audio resources for USB and I2S
on tile[0]: port p_for_mclk_count               = XS1_PORT_16B;
on tile[0]: port p_for_mclk_in                  = XS1_PORT_1D;
on tile[0]: clock usb_mclk_in_clk               = XS1_CLKBLK_2;

on tile[1]: buffered out port:32 p_i2s_dac[]    = {
                                                PORT_I2S_DAC0, PORT_I2S_DAC1
                                                };   /* I2S Data-line(s) */
on tile[1]: buffered in port:32 p_i2s_adc[]     = {
                                                PORT_I2S_ADC0, PORT_I2S_ADC1
                                                };   /* I2S Data-line(s) */
on tile[1]: buffered out port:32 p_lrclk        = PORT_I2S_LRCLK;    /* I2S Bit-clock */
on tile[1]: buffered out port:32 p_bclk         = PORT_I2S_BCLK;     /* I2S L/R-clock */
on tile[1]: clock clk_audio_mclk                = XS1_CLKBLK_1;
on tile[1]: clock clk_audio_bclk                = XS1_CLKBLK_2;
on tile[1]: port p_mclk_in                      = PORT_MCLK_IN;

// I2C resources (defined in audiohw.c)
extern port p_scl;
extern port p_sda;

// GPIO resources
on tile[0]: in port p_mc_buttons                = XS1_PORT_4E;      // 3 buttons on MC board
on tile[0]: out port p_mc_leds                  = XS1_PORT_4F;      // 4 LEDs on MC board
on tile[0]: out buffered port:32 p_neopixel     = XS1_PORT_1A;      // PLL_SYNC in
on tile[0]: clock cb_neo                        = XS1_CLKBLK_3;
on tile[1]: port p_uart_tx                      = PORT_MIDI_OUT; // Bit 0
on tile[1]: port p_qadc[]                       = {
                                                PORT_I2S_ADC2, PORT_I2S_ADC3
                                                }; // Sets which pins are to be used (channels 0..n)


int main() {
    // Cross tile comms    
    chan c_aud;
    chan c_qadc;
         
    interface i2c_master_if i2c[2];
    interface uart_tx_if i_uart_tx;
    interface adsp_control_if i_adsp_control;
   
    par
    {
        on tile[0]: unsafe{
            board_setup(); // Turns on power for 316MC board

            // Local interfaces
            input_gpio_if i_gpio_mc_buttons[NUM_BUTTONS];
            output_gpio_if i_gpio_mc_leds[NUM_LEDS];

            // XUA chans
            chan c_ep_out[2];
            chan c_ep_in[3];
            chan c_sof;
            chan c_aud_ctl;

            // Declare enpoint tables
            XUD_EpType epTypeTableOut[2] = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};
            XUD_EpType epTypeTableIn[3] = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO, XUD_EPTYPE_ISO};

            /* Connect master-clock clock-block to clock-block pin */
            set_clock_src(usb_mclk_in_clk, p_for_mclk_in);           /* Clock clock-block from mclk pin */
            set_port_clock(p_for_mclk_count, usb_mclk_in_clk);       /* Clock the "count" port from the clock block */
            start_clock(usb_mclk_in_clk);                            /* Set the clock off running */



            par{
                XUD_Main(   c_ep_out, 2, c_ep_in, 3,
                            c_sof, epTypeTableOut, epTypeTableIn,
                            XUD_SPEED_HS, XUD_PWR_SELF);
                XUA_Endpoint0(c_ep_out[0], c_ep_in[0], c_aud_ctl, null, null, null, null);
                XUA_Buffer(c_ep_out[1], c_ep_in[2], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);

                gpio_control_task(  i_uart_tx,
                                    c_qadc,
                                    i_adsp_control,
                                    p_neopixel, cb_neo,
                                    i2c[1],
                                    i_gpio_mc_buttons,
                                    i_gpio_mc_leds);

                [[distribute]]
                output_gpio(i_gpio_mc_leds, NUM_LEDS, p_mc_leds, null);
                [[distribute]]
                input_gpio(i_gpio_mc_buttons, NUM_BUTTONS, p_mc_buttons, null);

                i2c_master(i2c, 2, p_scl, p_sda, 100);

            }
        }

        on tile[1]: {
            // Local comms
            output_gpio_if i_gpio_tx[1];

            // Setup audio hardware on MC board
            unsafe{ i_i2c_client = i2c[0];}

            /* Quasi-ADC setup parameters */
            const unsigned capacitor_pf = ADC_CAPACITOR_PF;
            const unsigned potentiometer_ohms = ADC_POTENTIOMETER_OHMS; // nominal maximum value end to end
            const unsigned resistor_series_ohms = ADC_SERIES_OHMS;
            const float v_rail = 3.3;
            const float v_thresh = 1.14;
            const unsigned convert_interval_ticks = 1 * XS1_TIMER_KHZ;
            const uint8_t auto_scale = 0;
            const adc_pot_config_t adc_config = {capacitor_pf,
                                                potentiometer_ohms,
                                                resistor_series_ohms,
                                                v_rail,
                                                v_thresh,
                                                convert_interval_ticks,
                                                auto_scale};

            par{
                XUA_AudioHub(c_aud, clk_audio_mclk, clk_audio_bclk, p_mclk_in, p_lrclk, p_bclk, p_i2s_dac, p_i2s_adc);
                app_dsp_main_local_control();
                gpio_control_slave(i_adsp_control);

                // QADC setup and task
                {
                    adc_pot_state_t adc_pot_state;
                    uint16_t state_buffer[ADC_POT_STATE_SIZE(NUM_ADC_POTS, ADC_LUT_SIZE, ADC_FILTER_DEPTH)];
                    adc_pot_init(NUM_ADC_POTS, ADC_LUT_SIZE, ADC_FILTER_DEPTH, ADC_HYSTERESIS, state_buffer, adc_config, adc_pot_state);
                    adc_pot_task(c_qadc, p_qadc, adc_pot_state);
                }

                // UART Tx and support GPIO
                {
                    [[combine]]
                    par{
                        uart_tx(i_uart_tx, null,
                            UART_BAUD_RATE, UART_PARITY_NONE, 8, 1,
                            i_gpio_tx[0]);
                        output_gpio(i_gpio_tx, 1, p_uart_tx, null);
                    }
                }
            }
        }
    }
    return 0;
}
