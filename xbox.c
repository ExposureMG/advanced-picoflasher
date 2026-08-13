/*
 * Copyright (c) 2022 Balázs Triszka <balika011@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "xbox.h"
#include "pins.h"

void xbox_init()
{
	spi_init(spi0, 28 * 1000 * 1000);

	gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);
	gpio_pull_up(SPI_MISO);
	gpio_set_function(SPI_CLK, GPIO_FUNC_SPI);
	gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);

	gpio_init(SPI_SS_N);
	gpio_put(SPI_SS_N, 1);
	gpio_set_dir(SPI_SS_N, GPIO_OUT);

	gpio_init(SMC_DBG_EN);
	gpio_put(SMC_DBG_EN, 0);
	gpio_set_dir(SMC_DBG_EN, GPIO_OUT);

	gpio_init(SMC_RST_XDK_N);
	gpio_put(SMC_RST_XDK_N, 1);
	gpio_set_dir(SMC_RST_XDK_N, GPIO_OUT);
}

bool xbox_smc_stopped = false;

void xbox_start_smc()
{
	gpio_put(SMC_DBG_EN, 0);
	gpio_put(SMC_RST_XDK_N, 0);

	sleep_ms(50);

	gpio_put(SMC_RST_XDK_N, 1);

	xbox_smc_stopped = false;
}


void xbox_stop_smc()
{
	gpio_put(SMC_DBG_EN, 0);

	sleep_ms(50);

	gpio_put(SPI_SS_N, 0);
	gpio_put(SMC_RST_XDK_N, 0);

	sleep_ms(50);

	gpio_put(SMC_DBG_EN, 1);
	gpio_put(SMC_RST_XDK_N, 1);

	sleep_ms(50);

	gpio_put(SPI_SS_N, 1);

	sleep_ms(50);

	xbox_smc_stopped = true;
}

static uint8_t lsb2msb[] =
{
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

uint32_t xbox_read_reg(uint8_t reg)
{
	uint8_t txbuf[] = {(reg << 2) | 1, 0xFF, 0x00, 0x00, 0x00, 0x00};
	uint8_t rxbuf[sizeof(txbuf)];

	for (int i = 0; i < sizeof(txbuf); i++)
		txbuf[i] = lsb2msb[txbuf[i]];

	gpio_put(SPI_SS_N, 0);

	spi_write_read_blocking(spi0, txbuf, rxbuf, sizeof(txbuf));

	gpio_put(SPI_SS_N, 1);

	for (int i = 0; i < sizeof(rxbuf); i++)
		rxbuf[i] = lsb2msb[rxbuf[i]];

	return *(uint32_t *)&rxbuf[2];
}

void xbox_write_reg(uint8_t reg, uint32_t val)
{

	uint8_t txbuf[] = {(reg << 2) | 2, 0x00, 0x00, 0x00, 0x00};

	*(uint32_t *)&txbuf[1] = val;

	for (int i = 0; i < sizeof(txbuf); i++)
		txbuf[i] = lsb2msb[txbuf[i]];

	gpio_put(SPI_SS_N, 0);

	spi_write_blocking(spi0, txbuf, sizeof(txbuf));

	gpio_put(SPI_SS_N, 1);
}

static uint32_t xbox_cached_flash_config = 0;

uint32_t xbox_get_flash_config()
{
	if (!xbox_smc_stopped) {
		xbox_cached_flash_config = 0;
		return 0;
	}
	xbox_cached_flash_config = xbox_read_reg(0);
	if ((xbox_cached_flash_config & 0xF0000000) == 0xC0000000) {
		xbox_cached_flash_config = 0xC0462002;
	}
	return xbox_cached_flash_config;
}

static uint16_t xbox_nand_get_status()
{
	return xbox_read_reg(0x04);
}

static void xbox_nand_clear_status()
{
	xbox_write_reg(0x04, xbox_read_reg(0x04));
}

static int xbox_nand_wait_ready(uint16_t timeout)
{
	do
	{
		if (!(xbox_nand_get_status() & 0x01))
			return 0;
	} while (timeout--);

	return 1;
}

int xbox_nand_read_block(uint32_t lba, uint8_t *buffer, uint8_t *spare)
{
	if (!xbox_smc_stopped || !xbox_cached_flash_config)
		return 0;

	xbox_nand_clear_status();

	xbox_write_reg(0x0C, lba << 9);

	xbox_write_reg(0x08, 0x03);

	if (xbox_nand_wait_ready(0x1000))
		return 0x8000 | xbox_nand_get_status();

	xbox_write_reg(0x0C, 0);

	uint8_t *end = buffer + 0x200;
	while (buffer < end)
	{
		xbox_write_reg(0x08, 0x00);

		*(uint32_t *) buffer = xbox_read_reg(0x10);
		buffer += 4;
	}

	end = spare + 0x10;
	while (spare < end)
	{
		xbox_write_reg(0x08, 0x00);

		*(uint32_t *)spare = xbox_read_reg(0x10);
		spare += 4;
	}

	return 0;
}

int xbox_nand_erase_block(uint32_t lba)
{
	if (!xbox_smc_stopped || !xbox_cached_flash_config)
		return 0;

	xbox_nand_clear_status();

	xbox_write_reg(0x00, xbox_read_reg(0x00) | 0x08);

	xbox_write_reg(0x0C, lba << 9);

	xbox_write_reg(0x08, 0xAA);
	xbox_write_reg(0x08, 0x55);
	xbox_write_reg(0x08, 0x05);

	if (xbox_nand_wait_ready(0x1000))
		return 0x8000 | xbox_nand_get_status();

	return 0;
}

int xbox_nand_write_block(uint32_t lba, uint8_t *buffer, uint8_t *spare)
{
	if (!xbox_smc_stopped || !xbox_cached_flash_config)
		return 0;

	int major = (xbox_cached_flash_config >> 17) & 3;
	int minor = (xbox_cached_flash_config >> 4) & 3;

	int blocksize = 0x4000;
	if (major >= 1)
	{
		if (minor == 2)
			blocksize = 0x20000;
		else if (minor == 3)
			blocksize = 0x40000;
	}

	int sectors_in_block = blocksize  / 0x200;

	// erase ereases `blocksize` bytes
	if (lba % sectors_in_block == 0)
	{
		int ret = xbox_nand_erase_block(lba);
		if (ret)
			return ret;
	}

	xbox_nand_clear_status();

	xbox_write_reg(0x0C, 0);

	uint8_t *end = buffer + 0x200;
	while (buffer < end)
	{
		xbox_write_reg(0x10, *(uint32_t *)buffer);

		xbox_write_reg(0x08, 0x01);

		buffer += 4;
	}

	end = spare + 0x10;
	while (spare < end)
	{
		xbox_write_reg(0x10, *(uint32_t *)spare);

		xbox_write_reg(0x08, 0x01);

		spare += 4;
	}

	if (xbox_nand_wait_ready(0x1000))
		return 0x8000 | xbox_nand_get_status();

	xbox_write_reg(0x0C, lba << 9);

	if (xbox_nand_wait_ready(0x1000))
		return 0x8000 | xbox_nand_get_status();

	xbox_write_reg(0x08, 0x55);
	xbox_write_reg(0x08, 0xAA);
	xbox_write_reg(0x08, 0x04);

	if (xbox_nand_wait_ready(0x1000))
		return 0x8000 | xbox_nand_get_status();

	return 0;
}

#define SD_OK (0)
#define SD_ERR_TIMEOUT (-1)
#define SD_ERR_BAD_RESPONSE (-2)
#define SD_ERR_CRC (-3)
#define SD_ERR_BAD_PARAM (-4)

static uint32_t xbox_emmc_get_ints()
{
	return xbox_read_reg(0x30);
}

static void xbox_emmc_clear_ints(uint32_t value)
{
	return xbox_write_reg(0x30, value);
}

static void xbox_emmc_clear_all_ints()
{
	xbox_emmc_clear_ints(xbox_emmc_get_ints());
}

static int xbox_emmc_wait_ints(uint32_t value, int timeout_ms)
{
	absolute_time_t wait_timeout = make_timeout_time_ms(timeout_ms);
	do
	{
		uint32_t ints = xbox_emmc_get_ints();
		if ((ints & value) == value)
		{
			return SD_OK;
		}
	} while (!time_reached(wait_timeout));
	return SD_ERR_TIMEOUT;
}

void xbox_emmc_execute(uint32_t reg_4, uint32_t reg_8, uint32_t reg_c)
{
	xbox_emmc_clear_all_ints();
	xbox_write_reg(0x04, reg_4);
	xbox_write_reg(0x08, reg_8);
	xbox_write_reg(0x0C, reg_c);
}

int xbox_emmc_init()
{
	xbox_write_reg(0x2C, xbox_read_reg(0x2C) | (1 << 24));
	absolute_time_t init_timeout = make_timeout_time_ms(5000);
	while (!time_reached(init_timeout))
	{
		if (xbox_read_reg(0x3C) & 0x1000000)
			break;
	}
	if (time_reached(init_timeout))
	{
		return SD_ERR_TIMEOUT;
	}
	return SD_OK;
}

static int xbox_emmc_deselect_card()
{
	xbox_emmc_execute(0, 0, 0x7000000);
	return xbox_emmc_wait_ints(1, 100);
}

static int xbox_emmc_read_cid_csd(uint8_t * buf, int is_cid)
{
	xbox_emmc_execute(0, 0xffff0000, is_cid ? 0x9010000 : 0xA010000);
	int ret = xbox_emmc_wait_ints(1, 100);
	if (!ret)
	{
		for (int i = 0x10; i < 0x20; i += 4)
		{
			uint32_t data = xbox_read_reg(i);
			memcpy(buf, &data, 4);
			buf += 4;
		}
	}
    return ret;
}

int xbox_emmc_read_cid(uint8_t * cid)
{
	return xbox_emmc_read_cid_csd(cid, 1);
}

int xbox_emmc_read_csd(uint8_t * csd)
{
	return xbox_emmc_read_cid_csd(csd, 0);
}

static int xbox_emmc_select_card()
{
	xbox_emmc_execute(0, 0xffff0000, 0x71a0000);
	return xbox_emmc_wait_ints(1, 100);
}

static int xbox_emmc_set_blocklen(int blocklen)
{
	xbox_emmc_execute(0x200, blocklen, 0x101a0000);
	return xbox_emmc_wait_ints(1, 100);
}

static int xbox_emmc_read_block_ext_csd(uint8_t * buf, int block, int is_block)
{
	int ret = xbox_emmc_select_card();
	if (ret)
		return ret;
	if (is_block)
	{
		ret = xbox_emmc_set_blocklen(0x200);
		if (ret)
		{
			xbox_emmc_deselect_card();
			return ret;
		}
	}
	xbox_emmc_execute(0x10200, block << 9, is_block ? 0x113a0010 : 0x83A0010);
	ret = xbox_emmc_wait_ints(0x21, 1500);
	if (!ret)
	{
		for (int i = 0; i < 0x200; i += 4)
		{
			uint32_t data = xbox_read_reg(0x20);
			memcpy(buf + i, &data, 4);
		}
	}
	xbox_emmc_deselect_card();
	return ret;
}

int xbox_emmc_read_ext_csd(uint8_t *ext_csd)
{
	return xbox_emmc_read_block_ext_csd(ext_csd, 0, 0);
}

int xbox_emmc_read_block(int lba, uint8_t *buf)
{
	return xbox_emmc_read_block_ext_csd(buf, lba, 1);
}

int xbox_emmc_write_block(int lba, uint8_t *buf)
{
	int ret = xbox_emmc_select_card();
	if (ret)
		return ret;
	ret = xbox_emmc_set_blocklen(0x200);
	if (ret)
		return ret;
	xbox_emmc_execute(0x10200, lba << 9, 0x183a0000);
	ret = xbox_emmc_wait_ints(1, 100);
	if (!ret)
	{
		for (int i = 0; i < 0x200; i += 4)
		{
			uint32_t data;
			memcpy(&data, buf + i, 4);
			xbox_write_reg(0x20, data);
		}
		ret = xbox_emmc_wait_ints(0x12, 1500);
	}
	xbox_emmc_deselect_card();
	return ret;
}
