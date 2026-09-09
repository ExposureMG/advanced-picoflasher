#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/sync.h>
#include <hardware/structs/ioqspi.h>
#include <hardware/structs/sio.h>
#include "tusb.h"

#include "mode_manager.h"
#include "pins.h"
#include "xbox.h"
#include "pio_jtag.h"

static flasher_mode_t current_mode = MODE_PICOFLASHER;

pio_jtag_inst_t jtag_inst = {
    .pio = pio0,
    .sm = 0,
    .initialized = false
};

// Note: get_bootsel_button() is provided by TinyUSB rp2040 family.c
extern bool get_bootsel_button(void);

void mode_manager_init(void) {
    current_mode = MODE_PICOFLASHER;
    xbox_init();
}

flasher_mode_t mode_manager_get_mode(void) {
    return current_mode;
}

void mode_manager_set_mode(flasher_mode_t mode) {
    if (mode == current_mode) return;

    // 1. Deinitialize previous hardware mode
    if (current_mode == MODE_PICOFLASHER) {
        xbox_deinit();
    } else if (current_mode == MODE_DIRTYJTAG) {
        deinit_jtag(&jtag_inst);
    }

    // 2. Disconnect USB to force host to re-enumerate
    tud_disconnect();
    sleep_ms(150);

    // 3. Switch active mode
    current_mode = mode;

    // 4. Initialize new hardware mode
    if (current_mode == MODE_PICOFLASHER) {
        xbox_init();
    } else if (current_mode == MODE_DIRTYJTAG) {
        init_jtag(&jtag_inst, 1000, PIN_TCK, PIN_TDI, PIN_TDO, PIN_TMS, PIN_RST, PIN_TRST);
    }

    // 5. Reconnect USB
    tud_connect();
}

void mode_manager_toggle_mode(void) {
    if (current_mode == MODE_PICOFLASHER) {
        mode_manager_set_mode(MODE_DIRTYJTAG);
    } else {
        mode_manager_set_mode(MODE_PICOFLASHER);
    }
}

void mode_manager_task(void) {
    // 1. Check BOOTSEL button with debouncing
    static uint32_t last_btn_check = 0;
    static uint32_t btn_press_start = 0;
    static bool btn_was_pressed = false;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (now - last_btn_check >= 20) {
        last_btn_check = now;
        bool pressed = get_bootsel_button();

        if (pressed && !btn_was_pressed) {
            btn_was_pressed = true;
            btn_press_start = now;
        } else if (!pressed && btn_was_pressed) {
            // Button released: if pressed for at least 50ms, toggle mode
            if (now - btn_press_start >= 50 && now - btn_press_start < 3000) {
                mode_manager_toggle_mode();
            }
            btn_was_pressed = false;
        }
    }

    // 2. Visual LED indication
    uint32_t phase = now % 1000;

#ifdef PIN_LED
    if (current_mode == MODE_PICOFLASHER) {
        // PicoFlasher mode: steady slow blink (500ms on, 500ms off)
        gpio_put(PIN_LED, phase < 500);
    } else {
        // DirtyJTAG mode: rapid double blink (50ms on, 100ms off, 50ms on, 800ms off)
        bool led_on = (phase < 50) || (phase >= 150 && phase < 200);
        gpio_put(PIN_LED, led_on);
    }
#endif
}
