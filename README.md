![PicoFlasher logo](https://raw.githubusercontent.com/X360Tools/PicoFlasher/master/picoflasher.png)

# PicoFlasher-custom

Open-source all-in-one **Xbox 360 NAND/eMMC flasher** and **glitch chip JTAG programmer** firmware for the Raspberry Pi Pico and Waveshare RP2040-Zero.

This custom firmware combines:
- **PicoFlasher**: Xbox 360 NAND SPI / eMMC reader and writer with dual debug UART bridges (`KER_DBG` and `SMC_DBG`).
- **pico-dirtyJtag**: High-speed PIO-driven DirtyJTAG adapter for programming CPLD / glitch chip timing files (.xsvf / .svf) via J-Runner (DirtyPico360), UrJTAG, or openFPGALoader.

Both tools share the exact same physical pin header on the Pico and can be switched dynamically on-the-fly without reflashing or unplugging!

---

## Operating Modes

### 1. PicoFlasher Mode (Default)
- **USB Device**: `VID 0x600D, PID 0x7001`
- **Interfaces**: 3x CDC Virtual COM Ports (PicoFlasher Protocol, `KER_DBG`, `SMC_DBG`)
- **Use Case**: Reading / writing Xbox 360 NAND (SPI) or 4GB Corona eMMC with J-Runner or AutoGG.
- **LED**: Steady slow blink.

### 2. DirtyJTAG Mode
- **USB Device**: `VID 0x1209, PID 0xC0CA` (with Microsoft OS 1.0 WinUSB descriptors)
- **Interfaces**: Vendor Class Bulk endpoints + 2x CDC UART bridges (`KER_DBG`, `SMC_DBG`)
- **Use Case**: Flashing timing files to glitch chips (CoolRunner, Matrix, Ace, etc.) via JTAG.
- **LED**: Rapid double-blink.

---

## How to Switch Modes

1. **BOOTSEL Button**: Tap the onboard **BOOTSEL** button on your Raspberry Pi Pico at any time to toggle between PicoFlasher and DirtyJTAG mode.
2. **Baud Rate 2400 Touch**: Opening any virtual serial port and setting baud to `2400` toggles the operating mode. (Setting `1200` baud resets into USB bootloader).
3. **Software Command**:
   - Over PicoFlasher serial: Send command `0x30` (`CMD_SWITCH_TO_DIRTYJTAG`).
   - Over DirtyJTAG vendor endpoint: Send command `0x09` (`CMD_SWITCH_TO_PICOFLASHER`).

---

## Wiring Reference

For detailed wiring diagrams and notes, see [pinout.md](pinout.md).

### NAND Flash / eMMC & JTAG Pins (Shared Header)

| Pico Pin | RP2040-Zero Pin | Xbox 360 (PicoFlasher Mode) | Glitch Chip (DirtyJTAG Mode) |
|:---|:---|:---|:---|
| **GP16** (Pin 21) | **GP0** | `SPI_MISO` | `TDI` |
| **GP17** (Pin 22) | **GP1** | `SPI_SS_N` | `TDO` (Pico) / `TMS` (Zero) |
| **GP18** (Pin 24) | **GP2** | `SPI_CLK` | `TCK` |
| **GP19** (Pin 25) | **GP3** | `SPI_MOSI` | `TMS` (Pico) / `TDO` (Zero) |
| **GP20** (Pin 26) | **GP4** | `SMC_DBG_EN` | `RST` |
| **GP21** (Pin 27) | **GP5** | `SMC_RST_XDK_N` | `TRST` |
| **GND** | **GND** | `GND` | `GND` |

### Kernel Debug UART (KER_DBG)

| Pico Pin | RP2040-Zero Pin | Xbox Header |
|:---|:---|:---|
| **GP0** (UART0_TX) | **GP12** (UART0_TX) | `KER_DBG_RXD` |
| **GP1** (UART0_RX) | **GP13** (UART0_RX) | `KER_DBG_TXD` |

### SMC Debug UART (SMC_DBG)

| Pico Pin | RP2040-Zero Pin | Xbox Header |
|:---|:---|:---|
| **GP4** (UART1_TX) | **GP8** (UART1_TX) | `SMC_DBG_RXD` |
| **GP5** (UART1_RX) | **GP9** (UART1_RX) | `SMC_DBG_TXD` |

---

## Building

Requires CMake and the Raspberry Pi Pico SDK (`PICO_SDK_PATH`).

```bash
# Build for standard Pico and RP2040 Zero:
./build_release.sh
```

---

## Acknowledgements

- **balika011** for the original PicoFlasher
- **15432** for eMMC SPI support
- **hax360 / kmx360 / Mate Kukri** for huge speed improvemets
- **Patrick Dussud & Jean Thomas** for DirtyJTAG / pico-dirtyJtag
