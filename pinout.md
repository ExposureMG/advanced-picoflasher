# PicoFlasher-custom Pinout & Wiring Reference

**PicoFlasher-custom** is an all-in-one firmware merging **PicoFlasher** (Xbox 360 NAND/eMMC flasher) and **pico-dirtyJtag** (CPLD/glitch chip JTAG programmer) for the Raspberry Pi Pico and Waveshare RP2040-Zero.

---

## Unified Pin Mapping

The physical pins on the RP2040 are shared between NAND SPI/eMMC and JTAG:

| Function | Standard Pico Pin | Waveshare RP2040-Zero | Xbox 360 Connection | Glitch Chip (JTAG) Connection |
|:---|:---|:---|:---|:---|
| **Data In / MISO** | **GP16** (Pin 21) | **GP0** | `SPI_MISO` | `TDI` |
| **CS / TDO / TMS** | **GP17** (Pin 22) | **GP1** | `SPI_SS_N` | `TDO` (Pico) / `TMS` (Zero) |
| **Clock** | **GP18** (Pin 24) | **GP2** | `SPI_CLK` | `TCK` |
| **Data Out / MOSI** | **GP19** (Pin 25) | **GP3** | `SPI_MOSI` | `TMS` (Pico) / `TDO` (Zero) |
| **Debug Enable / Reset** | **GP20** (Pin 26) | **GP4** | `SMC_DBG_EN` | `RST` |
| **SMC Reset / TRST** | **GP21** (Pin 27) | **GP5** | `SMC_RST_XDK_N` | `TRST` |
| **Ground** | **GND** (Pin 23/38) | **GND** | `GND` | `GND` |
| **Power (Optional)** | **3V3 OUT** (Pin 36) | **3V3** | — | `VCC` (3.3V) |

---

## Debug UART Bridges

Both Kernel Debug and SMC Debug UARTs remain active and bridged across USB in both modes:

### Kernel Debug UART (KER_DBG)
| Pico Pin | RP2040-Zero Pin | Xbox 360 Header | Notes |
|:---|:---|:---|:---|
| **GP0** (UART0_TX) | **GP12** (UART0_TX) | `KER_DBG_RXD` | Transmits commands to console |
| **GP1** (UART0_RX) | **GP13** (UART0_RX) | `KER_DBG_TXD` | Receives kernel debug text from console |

### SMC Debug UART (SMC_DBG)
| Pico Pin | RP2040-Zero Pin | Xbox 360 Header | Notes |
|:---|:---|:---|:---|
| **GP4** (UART1_TX) | **GP8** (UART1_TX) | `SMC_DBG_RXD` | Transmits to Southbridge SMC |
| **GP5** (UART1_RX) | **GP9** (UART1_RX) | `SMC_DBG_TXD` | Receives SMC debug / POST log output |

---

## Operating Modes & How to Switch

PicoFlasher-custom runs in two distinct USB profiles to maintain 100% compatibility with official desktop tools:

### Mode 0: PicoFlasher Mode (Default)
- **USB Device**: `VID = 0x600D, PID = 0x7001`
- **Interfaces**: 3x CDC Virtual COM Ports
  - COM Port 1: PicoFlasher protocol (NAND read/write, eMMC detect/read/write)
  - COM Port 2: Kernel Debug UART bridge (115200 baud)
  - COM Port 3: SMC Debug UART bridge (115200 baud)
- **LED Indicator**: Slow steady blink (500ms on, 500ms off)
- **Supported Tools**: J-Runner with Extras, AutoGG, custom NAND scripts.

### Mode 1: DirtyJTAG Mode
- **USB Device**: `VID = 0x1209, PID = 0xC0CA`
- **Interfaces**: 
  - Interface 0: Vendor Class Bulk IN/OUT (DirtyJTAG probe with Microsoft OS 1.0 WinUSB auto-binding)
  - Interface 1 & 2: Kernel Debug and SMC Debug UART bridges
- **LED Indicator**: Rapid double-blink pattern
- **Supported Tools**: J-Runner (DirtyPico360), UrJTAG, openFPGALoader, xc3sprog, pyusb scripts.

---

## Mode Switching Methods

1. **BOOTSEL Button (Hardware Toggle)**:
   - Tap the onboard **BOOTSEL** button on your Raspberry Pi Pico for ~0.1 to 1 second.
   - The device will disconnect from USB, switch hardware modes, and re-enumerate as the other tool immediately. No unplugging or firmware reflashing needed!

2. **Baud Rate Touch (2400 Baud)**:
   - Opening any of the virtual COM ports and setting the baud rate to `2400` toggles the operating mode.
   - Setting the baud rate to `1200` resets the Pico into USB mass-storage bootloader mode.

3. **Software Protocol Commands**:
   - Over PicoFlasher CDC 0: Send command byte `0x30` (`CMD_SWITCH_TO_DIRTYJTAG`).
   - Over DirtyJTAG Vendor EP: Send command byte `0x09` (`CMD_SWITCH_TO_PICOFLASHER`).
