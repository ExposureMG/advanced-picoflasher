#pragma once

#include <stdint.h>

enum {
	GET_VERSION		= 0x00,
	GET_FLASH_CONFIG	= 0x01,
	READ_FLASH		= 0x02,
	WRITE_FLASH		= 0x03,
	READ_FLASH_STREAM	= 0x04,
	ERASE_FLASH		= 0x05,

	SET_SMC_WORKAROUND	= 0x20,
	STOP_SMC		= 0x21,
	START_SMC		= 0x22,

	CMD_SWITCH_TO_DIRTYJTAG	= 0x30,
	CMD_GET_CURRENT_MODE	= 0x31,

	EMMC_DETECT		= 0x50,
	EMMC_INIT		= 0x51,
	EMMC_GET_CID		= 0x52,
	EMMC_GET_CSD		= 0x53,
	EMMC_GET_EXT_CSD	= 0x54,
	EMMC_READ		= 0x55,
	EMMC_READ_STREAM	= 0x56,
	EMMC_WRITE		= 0x57,

	REBOOT_TO_BOOTLOADER	= 0xFE,
};

struct __attribute__((packed)) cmd
{
	uint8_t		cmd;
	uint32_t	lba;
};
