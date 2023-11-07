/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "midi_uart_lib.h"
#include "pio_midi_uart_lib.h"
#include "midi_device_multistream.h"
//--------------------------------------------------------------------+
// This program routes 5-pin DIN MIDI IN signals A & B to USB MIDI
// virtual cables 0 & 1 on the USB MIDI Bulk IN endpoint. It also
// routes MIDI data from USB MIDI virtual cables 0-5 on the USB MIDI
// Bulk OUT endpoint to the 5-pin DIN MIDI OUT signals A-F.
// The Pico board's LED blinks in a pattern depending on the Pico's
// USB connection state (See below).
//--------------------------------------------------------------------+


//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

static void led_blinking_task(void);
static void midi_task(void);

// UART selection Pin mapping. You can move these for your design if you want to
// Make sure all these values are consistent with your choice of midi_uart
#define MIDI_UART_NUM 1
const uint MIDI_UART_TX_GPIO = 4;
const uint MIDI_UART_RX_GPIO = 5;

static void *midi_uart_instance;


static void* midi_uarts[4]; // MIDI IN A, B, C, D and MIDI OUT A, B, C, D
//static void* midi_outs[2];  // MIDI OUT E-F

// MIDI UART pin usage (Move them if you want to)
static const uint MIDI_OUT_A_GPIO = 10;
static const uint MIDI_IN_A_GPIO = 6;
static const uint MIDI_OUT_B_GPIO = 11;
static const uint MIDI_IN_B_GPIO = 7;
static const uint MIDI_OUT_C_GPIO = 12;
static const uint MIDI_IN_C_GPIO = 8;
static const uint MIDI_OUT_D_GPIO = 13;
static const uint MIDI_IN_D_GPIO = 9;
//static const uint MIDI_OUT_E_GPIO = 0;
//static const uint MIDI_OUT_F_GPIO = 4;

//static const uint MIDI_OUT_C_GPIO = 10;
//static const uint MIDI_OUT_D_GPIO = 18;
//static const uint MIDI_OUT_E_GPIO = 3;
//static const uint MIDI_OUT_F_GPIO = 27;
/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  
  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

// Hardware UART
    midi_uart_instance = midi_uart_configure(MIDI_UART_NUM, MIDI_UART_TX_GPIO, MIDI_UART_RX_GPIO);
//    printf("Configured MIDI UART %u for %u baud\r\n", MIDI_UART_NUM, MIDI_UART_LIB_BAUD_RATE);
  
// Create the MIDI UARTs and MIDI OUTs
  midi_uarts[0] = pio_midi_uart_create(MIDI_OUT_A_GPIO, MIDI_IN_A_GPIO);
  midi_uarts[1] = pio_midi_uart_create(MIDI_OUT_B_GPIO, MIDI_IN_B_GPIO);
  midi_uarts[2] = pio_midi_uart_create(MIDI_OUT_C_GPIO, MIDI_IN_C_GPIO);
  midi_uarts[3] = pio_midi_uart_create(MIDI_OUT_D_GPIO, MIDI_IN_D_GPIO);
//  midi_outs[0] = pio_midi_out_create(MIDI_OUT_E_GPIO);
//  midi_outs[1] = pio_midi_out_create(MIDI_OUT_F_GPIO);
//  midi_outs[2] = pio_midi_out_create(MIDI_OUT_E_GPIO);
//  midi_outs[3] = pio_midi_out_create(MIDI_OUT_F_GPIO);
  printf("4-IN 4-OUT USB MIDI Device adapter\r\n");
  // 
  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();
    midi_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// MIDI Task
//--------------------------------------------------------------------+
static void poll_midi_uart_rx(bool connected)
{
    uint8_t rx[48];
    // Pull any bytes received on the MIDI UART out of the receive buffer and
    // send them out via USB MIDI on virtual cable 0
    uint8_t nread = midi_uart_poll_rx_buffer(midi_uart_instance, rx, sizeof(rx));
    if (nread > 0 && connected)
    {
        uint32_t nwritten = tud_midi_stream_write(0, rx, nread);
        if (nwritten != nread) {
            TU_LOG1("Warning: Dropped %lu bytes receiving from UART MIDI In\r\n", nread - nwritten);
        }
    }
}


static void poll_midi_uarts_rx(bool connected)
{
    uint8_t rx[48];
    // Pull any bytes received on the MIDI UART out of the receive buffer and
    // send them out via USB MIDI on virtual cable 0
    for (uint8_t cable = 0; cable < 4; cable++) {
        uint8_t nread = pio_midi_uart_poll_rx_buffer(midi_uarts[cable], rx, sizeof(rx));

        if (nread > 0 && connected)
        {
            uint32_t nwritten = tud_midi_stream_write(cable+2, rx, nread);
            if (nwritten != nread) {
                TU_LOG1("Warning: Dropped %lu bytes receiving from UART MIDI In %c\r\n", nread - nwritten, 'A'+cable);
            }
        }
    }
    
}

static void poll_usb_rx(bool connected)
{
    // device must be attached and have the endpoint ready to receive a message
    if (!connected)
    {
        return;
    }
    uint8_t rx[48];
    uint8_t cable_num;
    uint8_t npushed = 0;
    uint32_t nread =  tud_midi_demux_stream_read(&cable_num, rx, sizeof(rx));
    while (nread > 0) {
        if (cable_num > 1) {
            // then it is PIO MIDI OUTS
            npushed = pio_midi_uart_write_tx_buffer(midi_uarts[cable_num-2], rx, nread);
        }
        else if (cable_num < 2) {
            // then it is UART MIDI OUT
            npushed = midi_uart_write_tx_buffer(midi_uart_instance,rx,nread);
        }
        else {
            TU_LOG1("Received a MIDI packet on cable %u", cable_num);
            npushed = 0;
            continue;
        }
        if (npushed != nread) {
            TU_LOG1("Warning: Dropped %lu bytes sending to MIDI Out Port %c\r\n", nread - npushed, 'A' + cable_num);
        }
        nread =  tud_midi_demux_stream_read(&cable_num, rx, sizeof(rx));
    }
}

static void drain_serial_port_tx_buffers()
{
    uint8_t cable;
    for (cable = 0; cable < 4; cable++) {
        pio_midi_uart_drain_tx_buffer(midi_uarts[cable]);
    }
//    for (cable = 5; cable < 7; cable++) {
//        pio_midi_out_drain_tx_buffer(midi_outs[cable-2]);
//    }
}
static void midi_task(void)
{
    bool connected = tud_midi_mounted();
    poll_midi_uart_rx(connected);
    poll_midi_uarts_rx(connected);
    poll_usb_rx(connected);
    midi_uart_drain_tx_buffer(midi_uart_instance);
    drain_serial_port_tx_buffers();
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
static void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
