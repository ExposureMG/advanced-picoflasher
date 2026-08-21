![PicoFlasher logo](https://raw.githubusercontent.com/X360Tools/PicoFlasher/master/picoflasher.png)

# PicoFlasher

Open source XBOX 360 NAND flasher firmware for Raspberry Pi Pico

## Wiring:

### Nand Flash or eMMC

| Pico  | RP2040 Zero | Xbox           |
| ----- | ----------- | -------------- |
| GP16  | GP0         | SPI_MISO       |
| GP17  | GP1         | SPI_SS_N       |
| GP18  | GP2         | SPI_CLK        |
| GP19  | GP3         | SPI_MOSI       |
| GP20  | GP4         | SMC_DBG_EN     |
| GP21  | GP5         | SMC_RST_XDK_N  |
| GND   | GND         | GND            |

### Kernel Debug UART

| Pico           | RP2040 Zero     | Xbox          |
| -------------- | --------------- | ------------- |
| GP0 (UART0_TX) | GP12 (UART0_TX) | KER_DBG_RXD   |
| GP1 (UART0_RX) | GP13 (UART0_RX) | KER_DBG_TXD   |

### SMC Debug UART

There is a second debug UART in the Southbridge that can be used by the SMC firmware to read/write bytes.
On most Retail PCBs only the TX pin is actually wired to to the debug headers, but RX can be accessed with
some modifications.
For simple debug output from SMC firmware, it is enough to only wire up SMC_DBG_TXD.

| Pico           | RP2040 Zero    | Xbox          |
| -------------- | -------------- | ------------- |
| GP4 (UART1_TX) | GP8 (UART1_TX) | SMC_DBG_RXD   |
| GP5 (UART1_RX) | GP9 (UART1_RX) | SMC_DBG_TXD   |

### ISD ChipCorder

Previous revisions of PicoFlasher had support for flashing the ISD ChipCorder used for Power On and Eject sounds on
Xbox 360 S consoles.

This is now possible purely in software, by running an application on the console itself, because the SPI interface for
the ISD ChipCorder is connected to the console's southbridge.

Please use:
- Under the Xbox 360 System Software: https://github.com/Byrom90/SonusGUI
- Under libxenon: https://github.com/buddyjojo/Xbox360-ISD

## Acknowledgements

- balika011 for the original PicoFlasher
- 15432 for eMMC SPI support
