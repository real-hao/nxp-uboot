// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 NXP
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/imx91_pins.h>
#include <asm/arch/mu.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch-mx7ulp/gpio.h>
#include <asm/mach-imx/syscounter.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <linux/delay.h>
#include <asm/arch/clock.h>
#include <asm/arch/ccm_regs.h>
#include <asm/arch/ddr.h>
#include <power/pmic.h>
#include <power/pf9453.h>
#include <asm/arch/trdc.h>

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_board_init(void)
{
	int ret;

	puts("Normal Boot\n");

	ret = ele_start_rng();
	if (ret)
		printf("Fail to start RNG: %d\n", ret);
}

void spl_dram_init(void)
{
	struct dram_timing_info *ptiming = &dram_timing;

	printf("DDR: %uMTS\n", ptiming->fsp_msg[0].drate);
	ddr_init(ptiming);
}

#if CONFIG_IS_ENABLED(DM_PMIC_PF9453)
int power_init_board(void)
{
	struct udevice *dev;
	int ret;
	unsigned int buck_val;

	ret = pmic_get("pmic@32", &dev);
	if (ret == -ENODEV) {
		puts("No pf9453@32\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* enable DVS control through PMIC_STBY_REQ */
	pmic_reg_write(dev, PF9453_BUCK2CTRL, 0x59);

	if (is_voltage_mode(VOLT_LOW_DRIVE)) {
		buck_val = 0x10; /* 0.8v for Low drive mode */
		printf("PMIC: Low Drive Voltage Mode\n");
	} else if (is_voltage_mode(VOLT_NOMINAL_DRIVE)) {
		buck_val = 0x14; /* 0.85v for Nominal drive mode */
		printf("PMIC: Nominal Voltage Mode\n");
	} else {
		buck_val = 0x18; /* 0.9v for Over drive mode */
		printf("PMIC: Over Drive Voltage Mode\n");
	}

	pmic_reg_write(dev, PF9453_BUCK2OUT, buck_val);

	/* set standby voltage to 0.65v */
	pmic_reg_write(dev, PF9453_BUCK2OUT_STBY, 0x4);

	/* 1.1v for LPDDR4 */
	pmic_reg_write(dev, PF9453_BUCK1OUT, 0x14);

	ret = pmic_reg_read(dev, PF9453_CONFIG1);
	if (ret < 0)
		return ret;
	/*The timer default is 8sec and too long, so change to 100ms.*/
	pmic_reg_write(dev, PF9453_CONFIG1, ~PF9453_RESETKEY_TIMER_MASK & ret);

	/* set WDOG_B_CFG to cold reset */
	pmic_reg_write(dev, PF9453_RESET_CTRL, 0xA0);
	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	timer_init();

	arch_cpu_init();

	board_early_init_f();

	spl_early_init();

	preloader_console_init();

	ret = imx9_probe_mu();
	if (ret) {
		printf("Fail to init ELE API\n");
	} else {
		printf("SOC: 0x%x\n", gd->arch.soc_rev);
		printf("LC: 0x%x\n", gd->arch.lifecycle);
	}

	clock_init_late();

	power_init_board();

	if (!is_voltage_mode(VOLT_LOW_DRIVE))
		set_arm_core_max_clk();

	/* Init power of mix */
	soc_power_init();

	/* Setup TRDC for DDR access */
	trdc_init();

	/* DDR initialization */
	spl_dram_init();

	/* Put M33 into CPUWAIT for following kick */
	ret = m33_prepare();
	if (!ret)
		printf("M33 prepare ok\n");

	board_init_r(NULL, 0);
}
