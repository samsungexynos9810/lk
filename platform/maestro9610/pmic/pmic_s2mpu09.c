/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <debug.h>
#include <sys/types.h>
#include <platform/speedy.h>
#include <platform/delay.h>
#include <platform/pmic_s2mpu09.h>

/*
 * S2MPU09 INT Register is Read & Clear type.
 * Must use first read value.
 * Also, first read poweronsrc is set before bootup.
 */
static unsigned char read_int_first = 0;
static unsigned char read_int1_org, read_int2_org;
static unsigned char read_pwronsrc, read_wtsr_smpl;

void pmic_enable_manual_reset (void)
{
	unsigned char reg;

	/* Disable Warm Reset */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, &reg);
	reg &= ~WRSTEN;
	reg |= MRSEL;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, reg);

	/* Enable Manual Reset */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL1, &reg);
	reg |= MRSTB_EN;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL1, reg);
}

void pmic_init (void)
{
	unsigned char reg;
	unsigned char capsel_val;

	speedy_init();

	/* prevent RTC oscillator from operating abnormally */
	speedy_read(S2MPU09_RTC_ADDR, S2MPU09_RTC_CAP_SEL, &capsel_val);
	if ((capsel_val & 0xF0) != 0xF0) {
		speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_ETC_OTP, &reg);
		reg |= (1 << 6);
		speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_ETC_OTP, reg);
		mdelay(300);
		speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_ETC_OTP, &reg);
		reg &= ~(1 << 6);
		speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_ETC_OTP, reg);
	}

	/* Disable Manual Reset */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL1, &reg);
	reg &= ~MRSTB_EN;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL1, reg);

	/* Enable Warm Reset */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, &reg);
	reg |= WRSTEN;
	reg &= ~MRSEL;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, reg);

	/* Enable AP warm reset detection */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, &reg);
	reg |= WRSTBIEN;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, reg);

	/* PERI 32kHz on, AP 32kHz on */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_RTC_BUF, &reg);
	reg |= (_32KHZPERI_EN | _32KHZAP_EN);
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_RTC_BUF, reg);

#if 1
	/* defined (CONFIG_MACH_MAESTRO9610) */
	/* Enable LCD power */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_LDO38_CTRL, &reg);
	reg |= S2MPU09_OUTPUT_ON_NORMAL;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_LDO38_CTRL, reg);

	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_LDO39_CTRL, &reg);
	reg |= S2MPU09_OUTPUT_ON_NORMAL;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_LDO39_CTRL, reg);
#endif
}

void display_pmic_rtc_time(void)
{
#ifndef CONFIG_SG_BINARY
	int i;
	u8 tmp;
	u8 time[NR_PMIC_RTC_CNT_REGS];

	speedy_read(S2MPU09_RTC_ADDR, S2MPU09_RTC_UPDATE, &tmp);
	tmp |= 0x1;
	speedy_write(S2MPU09_RTC_ADDR, S2MPU09_RTC_UPDATE, tmp);
	u_delay(40);

	for (i = 0; i < NR_PMIC_RTC_CNT_REGS; i++)
		speedy_read(S2MPU09_RTC_ADDR, (S2MPU09_RTC_SEC + i), &time[i]);

	printf("RTC TIME: %d-%02d-%02d %02d:%02d:%02d(0x%02x)%s\n",
		time[PMIC_RTC_YEAR] + 2000, time[PMIC_RTC_MONTH],
		time[PMIC_RTC_DATE], time[PMIC_RTC_HOUR] & 0x1f, time[PMIC_RTC_MIN],
		time[PMIC_RTC_SEC], time[PMIC_RTC_WEEK],
		time[PMIC_RTC_HOUR] & (1 << 6) ? "PM" : "AM");
#endif
}

int get_pmic_rtc_time(char *buf)
{
#ifndef CONFIG_SG_BINARY
	int i;
	u8 tmp;
	u8 time[NR_PMIC_RTC_CNT_REGS];

	speedy_read(S2MPU09_RTC_ADDR, S2MPU09_RTC_UPDATE, &tmp);
	tmp |= 0x1;
	speedy_write(S2MPU09_RTC_ADDR, S2MPU09_RTC_UPDATE, tmp);
	u_delay(40);

	for (i = 0; i < NR_PMIC_RTC_CNT_REGS; i++)
		speedy_read(S2MPU09_RTC_ADDR, (S2MPU09_RTC_SEC + i), &time[i]);

	printf("RTC TIME: %d-%02d-%02d %02d:%02d:%02d(0x%02x)%s\n",
		time[PMIC_RTC_YEAR] + 2000, time[PMIC_RTC_MONTH],
		time[PMIC_RTC_DATE], time[PMIC_RTC_HOUR] & 0x1f, time[PMIC_RTC_MIN],
		time[PMIC_RTC_SEC], time[PMIC_RTC_WEEK],
		time[PMIC_RTC_HOUR] & (1 << 6) ? "PM" : "AM");

	sprintf(buf, "%04d%02d%02d%02d%02d%02d%s",
		time[PMIC_RTC_YEAR] + 2000, time[PMIC_RTC_MONTH],
		time[PMIC_RTC_DATE], time[PMIC_RTC_HOUR] & 0x1f, time[PMIC_RTC_MIN],
		time[PMIC_RTC_SEC], time[PMIC_RTC_HOUR] & (1 << 6) ? "PM" : "AM");

	return 0;
#else
	buf[0] = 0;
	return -1;
#endif
}

void read_pmic_info_s2mpu09 (void)
{
	/* read_poweronsrc, read_wtsr_smpl value are being used in other function */
	unsigned char read_int1, read_int2, read_offsrc, read_ctrl1, read_ctrl3;
	unsigned char read_rtc_buf;
#if 1
	/* defined (CONFIG_MACH_MAESTRO9610) */
	unsigned char read_ldo38_ctrl, read_ldo39_ctrl;
#endif

	/* read PMIC PM */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_INT1, &read_int1);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_INT2, &read_int2);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_PWRONSRC, &read_pwronsrc);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_OFFSRC, &read_offsrc);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_RTC_BUF, &read_rtc_buf);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL1, &read_ctrl1);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, &read_ctrl3);
#if 1
	/* defined (CONFIG_MACH_MAESTRO9610) */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_LDO38_CTRL, &read_ldo38_ctrl);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_LDO39_CTRL, &read_ldo39_ctrl);
#endif

	/* read PMIC RTC */
	speedy_read(S2MPU09_RTC_ADDR, S2MPU09_RTC_WTSR_SMPL, &read_wtsr_smpl);

	printf("S2MPU09_PM_INT1: 0x%x\n", read_int1);
	printf("S2MPU09_PM_INT2: 0x%x\n", read_int2);
	printf("S2MPU09_PM_PWRONSRC: 0x%x\n", read_pwronsrc);
	printf("S2MPU09_PM_OFFSRC: 0x%x\n", read_offsrc);
	printf("S2MPU09_PM_RTC_BUF: 0x%x\n", read_rtc_buf);
	printf("S2MPU09_PM_CTRL1: 0x%x\n", read_ctrl1);
	printf("S2MPU09_PM_CTRL3: 0x%x\n", read_ctrl3);
	printf("S2MPU09_RTC_WTSR_SMPL: 0x%x\n", read_wtsr_smpl);
#if 1
	/* defined (CONFIG_MACH_MAESTRO9610) */
	printf("S2MPU09_PM_LDO38_CTRL: 0x%x\n", read_ldo38_ctrl);
	printf("S2MPU09_PM_LDO39_CTRL: 0x%x\n", read_ldo39_ctrl);
#endif

	if (!read_int_first) {
		read_int_first = 1;
		read_int1_org = read_int1;
		read_int2_org = read_int2;
	} else {
		printf("S2MPU09_PM_INT1:(First read val): 0x%x\n", read_int1_org);
		printf("S2MPU09_PM_INT2:(First read val): 0x%x\n", read_int2_org);
	}

	display_pmic_rtc_time();
}

int chk_smpl_wtsr_s2mpu09(void)
{
	int ret = PMIC_DETECT_NONE;

	if (!read_int_first)
		read_pmic_info_s2mpu09();

	if ((read_pwronsrc & (1 << 7)) && (read_int2_org & (1 << 5)) && !(read_int1_org & (1 << 7))) {
		/* WTSR detect condition - WTSR_ON && WTSR_INT && ! MRB_INT */
		printf("chk_smpl_wtsr: WTSR detected\n");
		ret = PMIC_DETECT_WTSR;
	} else if ((read_pwronsrc & (1 << 6)) && (read_int2_org & (1 << 3)) && (read_wtsr_smpl & (1 << 7))) {
		/* SMPL detect condition - SMPL_ON && SMPL_INT && SMPL_EN */
		printf("chk_smpl_wtsr: SMPL detected\n");
		ret = PMIC_DETECT_SMPL;
	}

#ifdef S2MPU09_PM_IGNORE_SMPL_DETECT
	if (ret == PMIC_DETECT_SMPL) {
		printf("chk_smpl_wtsr: Ignore SMPL detection\n");
		ret = PMIC_DETECT_SMPL_IGNORE;
	}
#endif
#ifdef S2MPU09_PM_IGNORE_WTSR_DETECT
	if (ret == PMIC_DETECT_WTSR) {
		printf("chk_smpl_wtsr: Ignore WTSR detection\n");
		ret = PMIC_DETECT_WTSR_IGNORE;
	}
#endif

	return ret;
}
