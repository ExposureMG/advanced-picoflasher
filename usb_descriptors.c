/*
 * Copyright (c) 2022 Balázs Triszka <balika011@gmail.com>
 * Copyright (c) 2020-2025 Patrick Dussud
 * Copyright (c) 2023 David Williams (davidthings)
 * Combined PicoFlasher & DirtyJTAG USB Descriptors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include "tusb.h"
#include "mode_manager.h"
#include "get_serial.h"

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

// Mode 0: PicoFlasher Device Descriptor
tusb_desc_device_t const desc_device_picoflasher = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x600D,
    .idProduct          = 0x7001,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// Mode 1: DirtyJTAG Device Descriptor
tusb_desc_device_t const desc_device_dirtyjtag = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x1209,
    .idProduct          = 0xC0CA,
    .bcdDevice          = 0x0111,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
uint8_t const *tud_descriptor_device_cb(void)
{
    if (mode_manager_get_mode() == MODE_DIRTYJTAG) {
        return (uint8_t const *)&desc_device_dirtyjtag;
    }
    return (uint8_t const *)&desc_device_picoflasher;
}

//--------------------------------------------------------------------+
// Configuration Descriptors
//--------------------------------------------------------------------+

// Mode 0: PicoFlasher Configuration Descriptor (3x CDC)
enum {
    PF_ITF_NUM_CDC_0 = 0,
    PF_ITF_NUM_CDC_0_DATA,
    PF_ITF_NUM_CDC_1,
    PF_ITF_NUM_CDC_1_DATA,
    PF_ITF_NUM_CDC_2,
    PF_ITF_NUM_CDC_2_DATA,
    PF_ITF_NUM_TOTAL
};

#define PF_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + 3 * TUD_CDC_DESC_LEN)

#define PF_EPNUM_CDC_0_NOTIF   0x81
#define PF_EPNUM_CDC_0_OUT     0x02
#define PF_EPNUM_CDC_0_IN      0x82

#define PF_EPNUM_CDC_1_NOTIF   0x83
#define PF_EPNUM_CDC_1_OUT     0x04
#define PF_EPNUM_CDC_1_IN      0x84

#define PF_EPNUM_CDC_2_NOTIF   0x85
#define PF_EPNUM_CDC_2_OUT     0x06
#define PF_EPNUM_CDC_2_IN      0x86

uint8_t const desc_fs_config_picoflasher[] = {
    TUD_CONFIG_DESCRIPTOR(1, PF_ITF_NUM_TOTAL, 0, PF_CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_CDC_DESCRIPTOR(PF_ITF_NUM_CDC_0, 4, PF_EPNUM_CDC_0_NOTIF, 8, PF_EPNUM_CDC_0_OUT, PF_EPNUM_CDC_0_IN, 64),
    TUD_CDC_DESCRIPTOR(PF_ITF_NUM_CDC_1, 5, PF_EPNUM_CDC_1_NOTIF, 8, PF_EPNUM_CDC_1_OUT, PF_EPNUM_CDC_1_IN, 64),
    TUD_CDC_DESCRIPTOR(PF_ITF_NUM_CDC_2, 6, PF_EPNUM_CDC_2_NOTIF, 8, PF_EPNUM_CDC_2_OUT, PF_EPNUM_CDC_2_IN, 64),
};

// Mode 1: DirtyJTAG Configuration Descriptor (Vendor Class + 2x CDC UART)
enum {
    DJ_ITF_NUM_PROBE = 0,
    DJ_ITF_NUM_CDC_1,
    DJ_ITF_NUM_CDC_1_DATA,
    DJ_ITF_NUM_CDC_2,
    DJ_ITF_NUM_CDC_2_DATA,
    DJ_ITF_NUM_TOTAL
};

#define DJ_PROBE_OUT_EP_NUM 0x01
#define DJ_PROBE_IN_EP_NUM  0x82

#define DJ_CDC_NOTIF_EP1_NUM 0x83
#define DJ_CDC_OUT_EP1_NUM   0x03
#define DJ_CDC_IN_EP1_NUM    0x84

#define DJ_CDC_NOTIF_EP2_NUM 0x85
#define DJ_CDC_OUT_EP2_NUM   0x05
#define DJ_CDC_IN_EP2_NUM    0x86

#define DJ_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_VENDOR_DESC_LEN + 2 * TUD_CDC_DESC_LEN)

uint8_t const desc_fs_config_dirtyjtag[] = {
    TUD_CONFIG_DESCRIPTOR(1, DJ_ITF_NUM_TOTAL, 0, DJ_CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_VENDOR_DESCRIPTOR(DJ_ITF_NUM_PROBE, 0, DJ_PROBE_OUT_EP_NUM, DJ_PROBE_IN_EP_NUM, 64),
    TUD_CDC_DESCRIPTOR(DJ_ITF_NUM_CDC_1, 4, DJ_CDC_NOTIF_EP1_NUM, 8, DJ_CDC_OUT_EP1_NUM, DJ_CDC_IN_EP1_NUM, 64),
    TUD_CDC_DESCRIPTOR(DJ_ITF_NUM_CDC_2, 5, DJ_CDC_NOTIF_EP2_NUM, 8, DJ_CDC_OUT_EP2_NUM, DJ_CDC_IN_EP2_NUM, 64),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    if (mode_manager_get_mode() == MODE_DIRTYJTAG) {
        return desc_fs_config_dirtyjtag;
    }
    return desc_fs_config_picoflasher;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

char const *string_desc_arr_picoflasher[] = {
    (const char[]){0x09, 0x04}, // 0: English (0x0409)
    "PicoFlasher",              // 1: Manufacturer
    "PicoFlasher Device",       // 2: Product
    usb_serial,                 // 3: Serial
    "PicoFlasher UART",         // 4: CDC 0 (Protocol)
    "KER_DBG UART",             // 5: CDC 1
    "SMC_DBG UART",             // 6: CDC 2
};

char const *string_desc_arr_dirtyjtag[] = {
    (const char[]){0x09, 0x04}, // 0: English (0x0409)
    "Jean THOMAS",              // 1: Manufacturer
    "DirtyJTAG",                // 2: Product
    usb_serial,                 // 3: Serial
    "KER_DBG UART",             // 4: CDC 0
    "SMC_DBG UART",             // 5: CDC 1
};

// Microsoft OS 1.0 Descriptors for automatic WinUSB binding
#define VENDOR_REQUEST_CODE 0x01
#define MS_OS_STRING_INDEX 0xEE

static const uint8_t ms_os_string[] = {
    0x12,                   // Length (18 bytes)
    TUSB_DESC_STRING,       // Type (String)
    'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0, // "MSFT100"
    VENDOR_REQUEST_CODE,    // Vendor Code
    0x00                    // Padding
};

// Compatible ID Feature Descriptor (WinUSB for Interface 0)
static const uint8_t ms_compat_id_desc[] = {
    // Header
    0x28, 0x00, 0x00, 0x00,                         // Length: 40 bytes
    0x00, 0x01,                                     // Version: 1.0
    0x04, 0x00,                                     // Compatibility Header Descriptor: 0x0004
    0x01,                                           // Number of sections: 1
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       // Reserved: 7 bytes
    // Section 1 (Interface 0: WinUSB)
    0x00,                                           // Interface number: 0
    0x01,                                           // Reserved
    'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,       // Compatible ID: "WINUSB\0\0"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Sub-compatible ID: unused
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00              // Reserved: 6 bytes
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;

    if (index == MS_OS_STRING_INDEX) {
        return (uint16_t const *)ms_os_string;
    }

    bool is_dirtyjtag = (mode_manager_get_mode() == MODE_DIRTYJTAG);
    char const **arr = is_dirtyjtag ? string_desc_arr_dirtyjtag : string_desc_arr_picoflasher;
    size_t arr_size = is_dirtyjtag ? (sizeof(string_desc_arr_dirtyjtag)/sizeof(string_desc_arr_dirtyjtag[0]))
                                   : (sizeof(string_desc_arr_picoflasher)/sizeof(string_desc_arr_picoflasher[0]));

    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], arr[0], 2);
        chr_count = 1;
    } else {
        if (!(index < arr_size)) return NULL;

        const char *str = arr[index];
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;

        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str;
}

// Handle Vendor Class Setup Requests (MS OS 1.0 Compatible ID)
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    if (stage != CONTROL_STAGE_SETUP) return true;

    if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_VENDOR &&
        request->bRequest == VENDOR_REQUEST_CODE &&
        request->wIndex == 0x0004) {
        return tud_control_xfer(rhport, request, (void*)ms_compat_id_desc, sizeof(ms_compat_id_desc));
    }

    return false;
}
