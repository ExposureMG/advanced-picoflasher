/*
 * Copyright (c) 2022 Balázs Triszka <balika011@gmail.com>
 * Copyright (c) 2020-2025 Patrick Dussud
 * Combined PicoFlasher & DirtyJTAG Firmware
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "tusb.h"

#include "xbox.h"
#include "pins.h"
#include "protocol.h"
#include "mode_manager.h"
#include "pio_jtag.h"
#include "cmd.h"
#include "get_serial.h"

#define CDC_PICO_FLASHER 0

extern pio_jtag_inst_t jtag_inst;

static inline uint8_t get_ker_dbg_cdc(void) {
	return (mode_manager_get_mode() == MODE_DIRTYJTAG) ? 0 : 1;
}

static inline uint8_t get_smc_dbg_cdc(void) {
	return (mode_manager_get_mode() == MODE_DIRTYJTAG) ? 1 : 2;
}

bool stream_emmc = false;
bool do_stream = false;
uint32_t stream_offset = 0;
uint32_t stream_end = 0;

void pico_flasher_stream(uint8_t cdc_id)
{
	if (do_stream)
	{
		if (stream_offset >= stream_end)
		{
			do_stream = false;
			return;
		}

		if (tud_cdc_n_write_available(cdc_id) < 4 + (stream_emmc ? 0x200 : 0x210))
			return;

		if (!stream_emmc)
		{
			static uint8_t buffer[4 + 0x210];
			uint32_t ret = xbox_nand_read_block(stream_offset, &buffer[4], &buffer[4 + 0x200]);
			*(uint32_t *)buffer = ret;
			if (ret == 0)
			{
				tud_cdc_n_write(cdc_id, buffer, sizeof(buffer));
				++stream_offset;
			}
			else
			{
				tud_cdc_n_write(cdc_id, &ret, 4);
				do_stream = false;
			}
		}
		else
		{
			static uint8_t buffer[4 + 0x200];
			uint32_t ret = xbox_emmc_read_block(stream_offset, &buffer[4]);
			*(uint32_t *)buffer = ret;
			if (ret == 0)
			{
				tud_cdc_n_write(cdc_id, buffer, sizeof(buffer));
				++stream_offset;
			}
			else
			{
				tud_cdc_n_write(cdc_id, &ret, 4);
				do_stream = false;
			}
		}
	}
}

static bool enable_smc_workaround = true;

static void pico_flasher_rx_cb(uint8_t cdc_id)
{
	uint32_t avilable_data = tud_cdc_n_available(cdc_id);

	uint32_t needed_data = sizeof(struct cmd);
	{
		uint8_t cmd;
		tud_cdc_n_peek(cdc_id, &cmd);
		if (cmd == WRITE_FLASH)
			needed_data += 0x210;
		else if (cmd == EMMC_WRITE)
			needed_data += 0x200;
	}

	if (avilable_data >= needed_data)
	{
		struct cmd cmd;

		uint32_t count = tud_cdc_n_read(cdc_id, &cmd, sizeof(cmd));
		if (count != sizeof(cmd))
			return;

		switch (cmd.cmd)
		{
		case GET_VERSION:
		{
			uint32_t ver = 4;
			tud_cdc_n_write(cdc_id, &ver, 4);
			break;
		}
		case GET_FLASH_CONFIG:
		{
			if (enable_smc_workaround)
				xbox_stop_smc();

			uint32_t fc = xbox_get_flash_config();
			tud_cdc_n_write(cdc_id, &fc, 4);
			break;
		}
		case READ_FLASH:
		{
			uint8_t buffer[0x210];
			uint32_t ret = xbox_nand_read_block(cmd.lba, buffer, &buffer[0x200]);
			tud_cdc_n_write(cdc_id, &ret, 4);
			if (ret == 0)
				tud_cdc_n_write(cdc_id, buffer, sizeof(buffer));
			break;
		}
		case WRITE_FLASH:
		{
			uint8_t buffer[0x210];
			uint32_t count = tud_cdc_n_read(cdc_id, &buffer, sizeof(buffer));
			if (count != sizeof(buffer))
				return;
			uint32_t ret = xbox_nand_write_block(cmd.lba, buffer, &buffer[0x200]);
			tud_cdc_n_write(cdc_id, &ret, 4);
			break;
		}
		case ERASE_FLASH:
		{
			uint32_t ret = xbox_nand_erase_block(cmd.lba);
			tud_cdc_n_write(cdc_id, &ret, 4);
			break;
		}
		case READ_FLASH_STREAM:
			stream_emmc = false;
			do_stream = true;
			stream_offset = 0;
			stream_end = cmd.lba;
			break;
		case SET_SMC_WORKAROUND:
			enable_smc_workaround = cmd.lba & 1;
			break;
		case STOP_SMC:
			xbox_stop_smc();
			break;
		case START_SMC:
			xbox_start_smc();
			break;
		case CMD_SWITCH_TO_DIRTYJTAG:
		{
			uint32_t ack = 1;
			tud_cdc_n_write(cdc_id, &ack, 4);
			tud_cdc_n_write_flush(cdc_id);
			sleep_ms(50);
			mode_manager_set_mode(MODE_DIRTYJTAG);
			return;
		}
		case CMD_GET_CURRENT_MODE:
		{
			uint32_t mode = (uint32_t)mode_manager_get_mode();
			tud_cdc_n_write(cdc_id, &mode, 4);
			break;
		}
		case EMMC_DETECT:
		{
			uint32_t fc = xbox_get_flash_config();
			int emmc_detect_result = (fc & 0xF0000000) == 0xC0000000;
			tud_cdc_n_write(cdc_id, &emmc_detect_result, 1);
			break;
		}
		case EMMC_INIT:
		{
			uint32_t ret = xbox_emmc_init();
			tud_cdc_n_write(cdc_id, &ret, 4);
			break;
		}
		case EMMC_GET_CID:
		{
			uint8_t cid_raw[16] = {0};
			xbox_emmc_read_cid(cid_raw);
			tud_cdc_n_write(cdc_id, cid_raw, sizeof(cid_raw));
			break;
		}
		case EMMC_GET_CSD:
		{
			uint8_t csd_raw[16] = {0};
			xbox_emmc_read_csd(csd_raw);
			tud_cdc_n_write(cdc_id, csd_raw, sizeof(csd_raw));
			break;
		}
		case EMMC_GET_EXT_CSD:
		{
			uint8_t ext_csd[512];
			xbox_emmc_read_ext_csd(ext_csd);
			tud_cdc_n_write(cdc_id, ext_csd, sizeof(ext_csd));
			break;
		}
		case EMMC_READ:
		{
			uint8_t buffer[0x200];
			int ret = xbox_emmc_read_block(cmd.lba, buffer);
			tud_cdc_n_write(cdc_id, &ret, 4);
			if (ret == 0)
				tud_cdc_n_write(cdc_id, buffer, sizeof(buffer));
			break;
		}
		case EMMC_READ_STREAM:
			stream_emmc = true;
			do_stream = true;
			stream_offset = 0;
			stream_end = cmd.lba;
			break;
		case EMMC_WRITE:
		{
			uint8_t buffer[0x200];
			uint32_t count = tud_cdc_n_read(cdc_id, &buffer, sizeof(buffer));
			if (count != sizeof(buffer))
				return;
			uint32_t ret = xbox_emmc_write_block(cmd.lba, buffer);
			tud_cdc_n_write(cdc_id, &ret, 4);
			break;
		}
		case REBOOT_TO_BOOTLOADER:
			reset_usb_boot(0, 0);
			break;
		}

		tud_cdc_n_write_flush(cdc_id);
	}
}

static void uart_bridge_line_coding_cb(uint8_t cdc_id, const cdc_line_coding_t *line_coding, uart_inst_t *uart);

static void uart_bridge_init(uint8_t cdc_id, uart_inst_t *uart, int tx_pin, int rx_pin)
{
	uart_init(uart, 115200);
	gpio_set_function(tx_pin, UART_FUNCSEL_NUM(uart, GPIO_FUNC_UART));
	gpio_set_function(rx_pin, UART_FUNCSEL_NUM(uart, GPIO_FUNC_UART));

	cdc_line_coding_t line_coding;
	tud_cdc_n_get_line_coding(cdc_id, &line_coding);
	uart_bridge_line_coding_cb(cdc_id, &line_coding, uart);
}

static void uart_bridge_line_coding_cb(uint8_t cdc_id, const cdc_line_coding_t *line_coding, uart_inst_t *uart)
{
	(void)cdc_id;
	static const uart_parity_t uart_parity_tusb_to_pico[] = {
		UART_PARITY_NONE,	// 0: None
		UART_PARITY_ODD,	// 1: Odd
		UART_PARITY_EVEN,	// 2: Even
		UART_PARITY_NONE,	// 3: Mark
		UART_PARITY_NONE,	// 4: Space
	};

	static const uint8_t uart_stop_bits_tusb_to_pico[] = {
		1,			// 0: 1 stop bit
		1,			// 1: 1.5 stop bits
		2,			// 2: 2 stop bits
	};

	uart_set_baudrate(uart, line_coding->bit_rate);
	uart_set_format(uart,
		line_coding->data_bits,
		uart_stop_bits_tusb_to_pico[line_coding->stop_bits],
		uart_parity_tusb_to_pico[line_coding->parity]);
}

static void uart_bridge_task(uint8_t cdc_id, uart_inst_t *uart)
{
	uint8_t tmp;
	for (int maxr = 64; tud_cdc_n_available(cdc_id) && uart_is_writable(uart) && maxr; --maxr) {
		tud_cdc_n_read(cdc_id, &tmp, 1);
		uart_write_blocking(uart, &tmp, 1);
	}
	for (int maxw = 64; uart_is_readable(uart) && tud_cdc_n_write_available(cdc_id) && maxw; --maxw) {
		uart_read_blocking(uart, &tmp, 1);
		tud_cdc_n_write(cdc_id, &tmp, 1);
	}
	tud_cdc_n_write_flush(cdc_id);
}

// DirtyJTAG vendor packet task
static uint8_t djtag_rx_buf[64];
static uint8_t djtag_tx_buf[64];

static void djtag_task(void)
{
	if (tud_vendor_available()) {
		uint32_t count = tud_vendor_read(djtag_rx_buf, sizeof(djtag_rx_buf));
		if (count > 0) {
			cmd_handle(&jtag_inst, djtag_rx_buf, count, djtag_tx_buf);
		}
	}
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t cdc_id)
{
	if (mode_manager_get_mode() == MODE_PICOFLASHER && cdc_id == CDC_PICO_FLASHER) {
		pico_flasher_rx_cb(cdc_id);
	}
}

void tud_cdc_tx_complete_cb(uint8_t cdc_id)
{
	(void)cdc_id;
}

void tud_cdc_line_coding_cb(uint8_t cdc_id, const cdc_line_coding_t *line_coding)
{
	if (enable_smc_workaround && xbox_smc_stopped)
		xbox_start_smc();

	if (line_coding->bit_rate == 1200) {
		rom_reset_usb_boot_extra(-1, 0, 0);
	} else if (line_coding->bit_rate == 2400) {
		mode_manager_toggle_mode();
		return;
	}

	if (cdc_id == get_ker_dbg_cdc())
		uart_bridge_line_coding_cb(cdc_id, line_coding, uart0);
	else if (cdc_id == get_smc_dbg_cdc())
		uart_bridge_line_coding_cb(cdc_id, line_coding, uart1);
}

int main(void)
{
#ifdef PIN_LED
	gpio_init(PIN_LED);
	gpio_set_dir(PIN_LED, GPIO_OUT);
#endif

	usb_serial_init();
	tusb_init();
	mode_manager_init();

	uart_bridge_init(get_ker_dbg_cdc(), uart0, UART0_TX, UART0_RX);
	uart_bridge_init(get_smc_dbg_cdc(), uart1, UART1_TX, UART1_RX);

	gpio_init(I2C1_SDA);
	gpio_init(I2C1_SCL);

	while (1)
	{
		tud_task();
		mode_manager_task();

		if (mode_manager_get_mode() == MODE_PICOFLASHER) {
			pico_flasher_stream(CDC_PICO_FLASHER);
		} else {
			djtag_task();
		}

		uart_bridge_task(get_ker_dbg_cdc(), uart0);
		uart_bridge_task(get_smc_dbg_cdc(), uart1);
	}

	return 0;
}
