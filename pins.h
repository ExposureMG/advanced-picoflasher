/*
 * Copyright (c) 2022 Balázs Triszka <balika011@gmail.com>
 * Combined PicoFlasher & DirtyJTAG pin definitions
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
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

// Xbox 360 SPI / SMC Pins
#define SPI_MISO 0
#define SPI_SS_N 1
#define SPI_CLK 2
#define SPI_MOSI 3
#define SMC_DBG_EN 4
#define SMC_RST_XDK_N 5

// JTAG Pins (aligned with SPI/SMC header)
#define PIN_TDI 0
#define PIN_TMS 1
#define PIN_TCK 2
#define PIN_TDO 3
#define PIN_RST 4
#define PIN_TRST 5

#ifndef PIN_LED
#define PIN_LED 29
#endif

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

// Xbox 360 SPI / SMC Pins
#define SPI_MISO 16
#define SPI_SS_N 17
#define SPI_CLK 18
#define SPI_MOSI 19
#define SMC_DBG_EN 20
#define SMC_RST_XDK_N 21

// JTAG Pins (aligned with SPI/SMC header)
#define PIN_TDI 26
#define PIN_TDO 27
#define PIN_TCK 28
#define PIN_TMS 22
#define PIN_RST 20
#define PIN_TRST 21

#ifndef PIN_LED
#ifdef PICO_DEFAULT_LED_PIN
#define PIN_LED PICO_DEFAULT_LED_PIN
#else
#define PIN_LED 25
#endif
#endif

#endif

#endif
