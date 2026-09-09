#ifndef _MODE_MANAGER_H_
#define _MODE_MANAGER_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MODE_PICOFLASHER = 0,
    MODE_DIRTYJTAG   = 1,
} flasher_mode_t;

void mode_manager_init(void);
flasher_mode_t mode_manager_get_mode(void);
void mode_manager_set_mode(flasher_mode_t mode);
void mode_manager_toggle_mode(void);
void mode_manager_task(void);

// Safe BOOTSEL button polling
bool get_bootsel_button(void);

#endif
