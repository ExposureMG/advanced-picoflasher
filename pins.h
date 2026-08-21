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

#ifndef __PINS_H__
#define __PINS_H__

#ifdef WAVESHARE_RP2040_ZERO // Pinout for RP2040 Zero

// KER_DBG
#define UART0_TX 12
#define UART0_RX 13

// SMBUS
#define I2C1_SDA 6
#define I2C1_SCL 7

// SMC_DBG
#define UART1_TX 8
#define UART1_RX 9

#define SPI_MISO 0
#define SPI_SS_N 1
#define SPI_CLK 2
#define SPI_MOSI 3
#define SMC_DBG_EN 4
#define SMC_RST_XDK_N 5

#else // Pinout for standard Pico

// KER_DBG
#define UART0_TX 0
#define UART0_RX 1

// SMBUS
#define I2C1_SDA 2
#define I2C1_SCL 3

// SMC_DBG
#define UART1_TX 4
#define UART1_RX 5

#define SPI_MISO 16
#define SPI_SS_N 17
#define SPI_CLK 18
#define SPI_MOSI 19
#define SMC_DBG_EN 20
#define SMC_RST_XDK_N 21

#endif

#endif
