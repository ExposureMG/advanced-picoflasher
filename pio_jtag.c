/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2025 Patrick Dussud
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
 */

#include <string.h>
#include <hardware/clocks.h>
#include "hardware/dma.h"
#include "pins.h"
#include "pio_jtag.h"
#include "jtag.pio.h"
#include "tusb.h"

#define DMA

static bool last_tdo = false;

#ifdef DMA
static int tx_dma_chan = -1;
static int rx_dma_chan = -1;
static dma_channel_config tx_c;
static dma_channel_config rx_c;

static void dma_init(void)
{
    if (tx_dma_chan == -1)
    {
        tx_dma_chan = dma_claim_unused_channel(true);
        tx_c = dma_channel_get_default_config(tx_dma_chan);
        channel_config_set_transfer_data_size(&tx_c, DMA_SIZE_8);
        channel_config_set_read_increment(&tx_c, true);
        channel_config_set_dreq(&tx_c, DREQ_PIO0_TX0);
        dma_channel_configure(
            tx_dma_chan,
            &tx_c,
            &pio0_hw->txf[0],
            NULL,
            0,
            false
        );

        rx_dma_chan = dma_claim_unused_channel(true);
        rx_c = dma_channel_get_default_config(rx_dma_chan);
        channel_config_set_transfer_data_size(&rx_c, DMA_SIZE_8);
        channel_config_set_write_increment(&rx_c, false);
        channel_config_set_read_increment(&rx_c, false);
        channel_config_set_dreq(&rx_c, DREQ_PIO0_RX0);
        dma_channel_configure(
            rx_dma_chan,
            &rx_c,
            NULL,
            &pio0_hw->rxf[0],
            0,
            false
        );
    }
}
#endif

void __time_critical_func(pio_jtag_write_blocking)(const pio_jtag_inst_t *jtag, const uint8_t *bsrc, size_t len) 
{
    size_t byte_length = (len + 7) >> 3;
    size_t last_shift = ((byte_length << 3) - len);
    size_t tx_remain = byte_length, rx_remain = last_shift ? byte_length : byte_length + 1;
    io_rw_8 *txfifo = (io_rw_8 *) &jtag->pio->txf[jtag->sm];
    io_rw_8 *rxfifo = (io_rw_8 *) &jtag->pio->rxf[jtag->sm];
    uint8_t x = 0;

    *(io_rw_32*)txfifo = len - 1;
#ifdef DMA
    if (byte_length > 4)
    {
        dma_init();
        channel_config_set_read_increment(&tx_c, true);
        channel_config_set_write_increment(&rx_c, false);
        dma_channel_set_config(rx_dma_chan, &rx_c, false);
        dma_channel_set_config(tx_dma_chan, &tx_c, false);
        dma_channel_transfer_to_buffer_now(rx_dma_chan, (void*)&x, rx_remain);
        dma_channel_transfer_from_buffer_now(tx_dma_chan, (void*)bsrc, tx_remain);
        while (dma_channel_is_busy(rx_dma_chan))
        {
            tud_task();
            tight_loop_contents();
        }
        __compiler_memory_barrier();
    }
    else
#endif
    {
        while (tx_remain || rx_remain) 
        {
            if (tx_remain && !pio_sm_is_tx_fifo_full(jtag->pio, jtag->sm))
            {
                *txfifo = *bsrc++;
                --tx_remain;
            }
            if (rx_remain && !pio_sm_is_rx_fifo_empty(jtag->pio, jtag->sm))
            {
                x = *rxfifo;
                --rx_remain;
            }
        }
    }
    last_tdo = !!(x & 1);
}

void __time_critical_func(pio_jtag_write_read_blocking)(const pio_jtag_inst_t *jtag, const uint8_t *bsrc, uint8_t *bdst, size_t len) 
{
    size_t byte_length = (len + 7) >> 3;
    size_t last_shift = ((byte_length << 3) - len);
    size_t tx_remain = byte_length, rx_remain = last_shift ? byte_length : byte_length + 1;
    io_rw_8 *txfifo = (io_rw_8 *) &jtag->pio->txf[jtag->sm];
    io_rw_8 *rxfifo = (io_rw_8 *) &jtag->pio->rxf[jtag->sm];

    *(io_rw_32*)txfifo = len - 1;
#ifdef DMA
    if (byte_length > 4)
    {
        dma_init();
        channel_config_set_read_increment(&tx_c, true);
        channel_config_set_write_increment(&rx_c, true);
        dma_channel_set_config(rx_dma_chan, &rx_c, false);
        dma_channel_set_config(tx_dma_chan, &tx_c, false);
        dma_channel_transfer_to_buffer_now(rx_dma_chan, (void*)bdst, rx_remain);
        dma_channel_transfer_from_buffer_now(tx_dma_chan, (void*)bsrc, tx_remain);
        while (dma_channel_is_busy(rx_dma_chan))
        {
            tud_task();
            tight_loop_contents();
        }
        __compiler_memory_barrier();
    }
    else
#endif
    {
        while (tx_remain || rx_remain) 
        {
            if (tx_remain && !pio_sm_is_tx_fifo_full(jtag->pio, jtag->sm))
            {
                *txfifo = *bsrc++;
                --tx_remain;
            }
            if (rx_remain && !pio_sm_is_rx_fifo_empty(jtag->pio, jtag->sm))
            {
                *bdst++ = *rxfifo;
                --rx_remain;
            }
        }
    }
    last_tdo = !!(bdst[-1] & 1);
}

uint8_t pio_jtag_write_tms_blocking(const pio_jtag_inst_t *jtag, bool tdi, bool tms, size_t len)
{
    pio_sm_set_enabled(jtag->pio, jtag->sm, false);
    gpio_set_function(jtag->pin_tdi, GPIO_FUNC_SIO);
    gpio_set_function(jtag->pin_tck, GPIO_FUNC_SIO);
    gpio_set_function(jtag->pin_tms, GPIO_FUNC_SIO);
    gpio_set_dir(jtag->pin_tdi, GPIO_OUT);
    gpio_set_dir(jtag->pin_tck, GPIO_OUT);
    gpio_set_dir(jtag->pin_tms, GPIO_OUT);
    gpio_set_dir(jtag->pin_tdo, GPIO_IN);

    gpio_put(jtag->pin_tdi, tdi);
    gpio_put(jtag->pin_tms, tms);

    uint8_t ret = 0;
    for (size_t i = 0; i < len; i++)
    {
        gpio_put(jtag->pin_tck, 0);
        sleep_us(1);
        gpio_put(jtag->pin_tck, 1);
        sleep_us(1);
        ret |= (gpio_get(jtag->pin_tdo) ? 1 : 0) << i;
    }
    gpio_put(jtag->pin_tck, 0);

    // restore PIO
    gpio_set_function(jtag->pin_tdi, GPIO_FUNC_PIO0);
    gpio_set_function(jtag->pin_tck, GPIO_FUNC_PIO0);
    pio_sm_set_enabled(jtag->pio, jtag->sm, true);

    return ret;
}

static void init_pins(uint pin_tck, uint pin_tdi, uint pin_tdo, uint pin_tms, uint pin_rst, uint pin_trst)
{
    if (pin_rst != 255)
    {
        gpio_init(pin_rst);
        gpio_set_dir(pin_rst, false);
        gpio_put(pin_rst, 0);
    }
    if (pin_trst != 255)
    {
        gpio_init(pin_trst);
        gpio_set_dir(pin_trst, true);
        gpio_put(pin_trst, 1);
    }

    gpio_init(pin_tms);
    gpio_put(pin_tms, 1);
    gpio_set_dir(pin_tms, true);

    gpio_init(pin_tdo);
    gpio_set_dir(pin_tdo, false);
}

void init_jtag(pio_jtag_inst_t* jtag, uint freq, uint pin_tck, uint pin_tdi, uint pin_tdo, uint pin_tms, uint pin_rst, uint pin_trst)
{
    init_pins(pin_tck, pin_tdi, pin_tdo, pin_tms, pin_rst, pin_trst);
    jtag->pin_tdi = pin_tdi;
    jtag->pin_tdo = pin_tdo;
    jtag->pin_tck = pin_tck;
    jtag->pin_tms = pin_tms;
    jtag->pin_rst = pin_rst;
    jtag->pin_trst = pin_trst;

    uint16_t clkdiv = 31;
    uint prog_offs = pio_add_program(jtag->pio, &djtag_tdo_program);
    jtag->prog_offset = prog_offs;

    pio_sm_config c = djtag_tdo_program_get_default_config(prog_offs);
    sm_config_set_out_pins(&c, pin_tdi, 1);
    sm_config_set_in_pins(&c, pin_tdo);
    sm_config_set_in_pin_count(&c, 1);
    sm_config_set_sideset_pins(&c, pin_tck);
    sm_config_set_out_shift(&c, false, true, 8);
    sm_config_set_in_shift(&c, false, true, 8);
    sm_config_set_clkdiv_int_frac(&c, clkdiv, 0);

    pio_sm_set_pins_with_mask(jtag->pio, jtag->sm, 0, (1u << pin_tck) | (1u << pin_tdi));
    pio_sm_set_pindirs_with_mask(jtag->pio, jtag->sm, (1u << pin_tck) | (1u << pin_tdi), (1u << pin_tck) | (1u << pin_tdi) | (1u << pin_tdo));
    pio_gpio_init(jtag->pio, pin_tdi);
    pio_gpio_init(jtag->pio, pin_tck);

    hw_set_bits(&jtag->pio->input_sync_bypass, 1u << pin_tdo);
    gpio_set_pulls(pin_tdo, false, true);
    pio_sm_init(jtag->pio, jtag->sm, prog_offs, &c);
    pio_sm_set_enabled(jtag->pio, jtag->sm, true);

    jtag_set_clk_freq(jtag, freq);
    jtag->initialized = true;
}

void deinit_jtag(pio_jtag_inst_t* jtag)
{
    if (!jtag->initialized) return;

#ifdef DMA
    if (tx_dma_chan != -1) {
        dma_channel_abort(tx_dma_chan);
        dma_channel_unclaim(tx_dma_chan);
        tx_dma_chan = -1;
    }
    if (rx_dma_chan != -1) {
        dma_channel_abort(rx_dma_chan);
        dma_channel_unclaim(rx_dma_chan);
        rx_dma_chan = -1;
    }
#endif

    pio_sm_set_enabled(jtag->pio, jtag->sm, false);
    pio_remove_program(jtag->pio, &djtag_tdo_program, jtag->prog_offset);

    uint pins[] = {jtag->pin_tck, jtag->pin_tdi, jtag->pin_tdo, jtag->pin_tms, jtag->pin_rst, jtag->pin_trst};
    for (size_t i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
        if (pins[i] != 255) {
            gpio_init(pins[i]);
            gpio_set_dir(pins[i], GPIO_IN);
            gpio_disable_pulls(pins[i]);
        }
    }

    jtag->initialized = false;
}

void jtag_set_clk_freq(const pio_jtag_inst_t *jtag, uint freq_khz) {
    uint clk_sys_freq_khz = clock_get_hz(clk_sys) / 1000;
    float divf = (float)clk_sys_freq_khz / (freq_khz * 4);
    uint16_t divider = (divf > (int)divf) ? (int)divf + 1 : (int)divf;
    divider = (divider < 2) ? 2 : divider;
    pio_sm_set_clkdiv_int_frac(jtag->pio, jtag->sm, divider, 0);
}

void jtag_transfer(const pio_jtag_inst_t *jtag, uint32_t length, const uint8_t* in, uint8_t* out)
{
    jtag_set_tms(jtag, false);
    if (out)
        pio_jtag_write_read_blocking(jtag, in, out, length);
    else
        pio_jtag_write_blocking(jtag, in, length);
}

uint8_t jtag_strobe(const pio_jtag_inst_t *jtag, uint32_t length, bool tms, bool tdi)
{
    if (length == 0)
        return jtag_get_tdo(jtag) ? 0xFF : 0x00;
    else
        return pio_jtag_write_tms_blocking(jtag, tdi, tms, length);
}

static uint8_t toggle_bits_out_buffer[4];
static uint8_t toggle_bits_in_buffer[4];

void jtag_set_tdi(const pio_jtag_inst_t *jtag, bool value)
{
    toggle_bits_out_buffer[0] = value ? 1u << 7 : 0;
}

void jtag_set_clk(const pio_jtag_inst_t *jtag, bool value)
{
    if (value)
    {
        pio_jtag_write_read_blocking(jtag, toggle_bits_out_buffer, toggle_bits_in_buffer, 1);
    }
}

bool jtag_get_tdo(const pio_jtag_inst_t *jtag)
{
    return last_tdo;
}
