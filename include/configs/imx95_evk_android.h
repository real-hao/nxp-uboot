/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef IMX95_EVK_ANDROID_H
#define IMX95_EVK_ANDROID_H

#define FSL_FASTBOOT_FB_DEV "mmc"

#undef CFG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOOTCOMMAND

#define CFG_EXTRA_ENV_SETTINGS		\
	"splashpos=m,m\0"			\
	"splashimage=0x9FFF0000\0" \
	"fdt_high=0xffffffffffffffff\0"		\
	"initrd_high=0xffffffffffffffff\0"	\
	"emmc_dev=0\0"\
	"sd_dev=1\0" \

/* Enable mcu firmware flash */
#ifdef CONFIG_FLASH_MCUFIRMWARE_SUPPORT
#define ANDROID_MCU_FRIMWARE_DEV_TYPE DEV_MMC
#define ANDROID_MCU_FIRMWARE_START 0x500000
#define ANDROID_MCU_OS_PARTITION_SIZE 0x40000
#define ANDROID_MCU_FIRMWARE_SIZE  0x20000
#define ANDROID_MCU_FIRMWARE_HEADER_STACK 0x2001e000
#endif

#define CFG_SYS_SPL_PTE_RAM_BASE    0x901F8000

#ifdef CONFIG_IMX_TRUSTY_OS
#define BOOTLOADER_RBIDX_OFFSET  0x3FE000
#define BOOTLOADER_RBIDX_START   0x3FF000
#define BOOTLOADER_RBIDX_LEN     0x08
#define BOOTLOADER_RBIDX_INITVAL 0
#endif

#ifdef CONFIG_IMX_TRUSTY_OS
#define AVB_RPMB
#define KEYSLOT_HWPARTITION_ID 2
#define KEYSLOT_BLKS             0x1FFF
#define NS_ARCH_ARM64 1
#endif

#endif /* IMX95_EVK_ANDROID_H */
