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
#include <ctype.h>
#include <stdlib.h>
#include <reg.h>
#include <libfdt.h>
#include <lib/bio.h>
#include <lib/console.h>
#include <lib/font_display.h>
#include <part_gpt.h>
#include <dev/boot.h>
#include <dev/rpmb.h>
#include <dev/usb/gadget.h>
#include <dev/phy-usb.h>
#include <platform/mmu/mmu_func.h>
#include <platform/exynos9610.h>
#include <platform/smc.h>
#include <platform/sfr.h>
#include <platform/ldfw.h>
#include <platform/charger.h>
#include <platform/ab_update.h>
#include <platform/secure_boot.h>
#include <platform/sizes.h>
#include <platform/fastboot.h>
#include <platform/bootimg.h>
#include <platform/fdt.h>
#include <platform/chip_id.h>
#include <platform/gpio.h>
#include <pit.h>
#include <dev/scsi.h>

/* Memory node */
#define SIZE_2GB (0x80000000)
#define MASK_1MB (0x100000 - 1)

#define BUFFER_SIZE 2048

#define REBOOT_MODE_RECOVERY	0xFF
#define REBOOT_MODE_FACTORY	0xFD
#define REBOOT_MODE_FASTBOOT	0xFC

void configure_ddi_id(void);
void arm_generic_timer_disable(void);

static char cmdline[AVB_CMD_MAX_SIZE];
static char verifiedbootstate[AVB_VBS_MAX_SIZE]="androidboot.verifiedbootstate=";

static void update_boot_reason(char *buf)
{
	u32 val = readl(CONFIG_RAMDUMP_REASON);
	unsigned int rst_stat = readl(EXYNOS9610_POWER_RST_STAT);

	if (rst_stat & WARM_RESET)
		snprintf(buf, 16, "hard_reset");
	else if (rst_stat & PIN_RESET)
		snprintf(buf, 16, "coldboot");
	else if (rst_stat & (LITTLE_WDT_RESET | BIG_WDT_RESET))
		snprintf(buf, 16, "watchdog");
	else if (rst_stat & SWRESET)
		if (val == RAMDUMP_SIGN_PANIC)
			snprintf(buf, 16, "kernel_panic");
		else
			snprintf(buf, 16, "reboot");
	else
		snprintf(buf, 16, "unknown");
}

struct bootargs_prop {
	char prop[128];
	char val[128];
};
static struct bootargs_prop prop[64] = { { {0, }, {0, } }, };
static int prop_cnt = 0;
extern char dtbo_idx[4];

static int bootargs_init(void)
{
	u32 i = 0;
	u32 len = 0;
	u32 cur = 0;
	u32 cnt = 0;
	u32 is_val = 0;
	char bootargs[BUFFER_SIZE];
	int len2;
	const char *np;
	int noff;
	int ret;

	ret = fdt_check_header(fdt_dtb);
	if (ret) {
		printf("libfdt fdt_check_header(): %s\n", fdt_strerror(ret));
		return ret;
	}

	noff = fdt_path_offset(fdt_dtb, "/chosen");
	if (noff >= 0) {
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len2);
		if (len2 >= 0) {
			memset(bootargs, 0, BUFFER_SIZE);
			memcpy(bootargs, np, len2);
		}
	}

	printf("\ndefault bootargs: %s\n", bootargs);

	len = strlen(bootargs);
	for (i = 0; i < len; i++) {
		if (bootargs[i] == '=') {
			is_val = 1;
			if (cnt > 0)
				prop[prop_cnt].val[cur++] = bootargs[i];
			else {
				cnt++;
				prop[prop_cnt].prop[cur++] = '\0';
				cur = 0;
			}
		} else if (bootargs[i] == ' ') {
			prop[prop_cnt].val[cur++] = '\0';
			is_val = 0;
			cur = 0;
			prop_cnt++;
			cnt = 0;
		} else {
			if (is_val)
				prop[prop_cnt].val[cur++] = bootargs[i];
			else
				prop[prop_cnt].prop[cur++] = bootargs[i];
		}
	}

	return 0;
}
static char *get_bootargs_val(const char *name)
{
       int i = 0;

       for (i = 0; i <= prop_cnt; i++) {
               if (strncmp(prop[i].prop, name, strlen(name)) == 0)
                       return prop[i].val;
       }

       return NULL;
}

static void update_val(const char *name, const char *val)
{
	int i = 0;

	for (i = 0; i <= prop_cnt; i++) {
		if (strncmp(prop[i].prop, name, strlen(name)) == 0) {
			sprintf(prop[i].val, "%s", val);
			return;
		}
	}
}

static void bootargs_update(void)
{
	int i = 0;
	int cur = 0;
	char bootargs[BUFFER_SIZE];

	memset(bootargs, 0, sizeof(bootargs));

	for (i = 0; i <= prop_cnt; i++) {
		if (0 == strlen(prop[i].val)) {
			sprintf(bootargs + cur, "%s", prop[i].prop);
			cur += strlen(prop[i].prop);
			snprintf(bootargs + cur, 2, " ");
			cur += 1;
		} else {
			sprintf(bootargs + cur, "%s=%s", prop[i].prop, prop[i].val);
			cur += strlen(prop[i].prop) + strlen(prop[i].val) + 1;
			snprintf(bootargs + cur, 2, " ");
			cur += 1;
		}
	}

	bootargs[cur] = '\0';

	printf("\nupdated bootargs: %s\n", bootargs);

	set_fdt_val("/chosen", "bootargs", bootargs);
}

static void remove_string_from_bootargs(const char *str)
{
	char bootargs[BUFFER_SIZE];
	const char *np;
	int noff;
	int bootargs_len;
	int str_len;
	int i;
	int flag = 0;

	noff = fdt_path_offset(fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &bootargs_len);

	str_len = strlen(str);

	for (i = 0; i <= bootargs_len - str_len; i++) {
		if(!strncmp(str, (np + i), str_len)) {
			flag = 1;
			break;
		}
	}

	if (!flag) {
		printf("%sis not in bootargs\n", str);
		return;
	}

	memset(bootargs, 0, BUFFER_SIZE);
	memcpy(bootargs, np, i);
	memcpy(bootargs + i, np + i + str_len, bootargs_len - i - str_len);

	fdt_setprop(fdt_dtb, noff, "bootargs", bootargs, strlen(bootargs) + 1);
}

static void set_bootargs(void)
{
	bootargs_init();

	/* update_val("console", "ttySAC0,115200"); */
	update_val("androidboot.dtbo_idx", dtbo_idx);

	bootargs_update();
}

static void set_usb_serialno(void)
{
	char str[BUFFER_SIZE];
	const char *np;
	int len;
	int noff;
	unsigned long tmp_serial_id = 0;

	tmp_serial_id = ((unsigned long)s5p_chip_id[1] << 32) | (s5p_chip_id[0]);

	printf("Set USB serial number in bootargs.(%016lx)\n", tmp_serial_id);

	noff = fdt_path_offset (fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
	snprintf(str, BUFFER_SIZE, "%s androidboot.serialno=%016lx",
						np, tmp_serial_id);
	fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
}

static void configure_dtb(void)
{
	char str[BUFFER_SIZE];
	char buf[16];
	u32 soc_ver = 0;
	unsigned long sec_dram_base = 0;
	unsigned int sec_dram_size = 0;
	unsigned long sec_dram_end = 0;
	unsigned long sec_pt_base = 0;
	unsigned int sec_pt_size = 0;
	unsigned long sec_pt_end = 0;
	u64 dram_size = *(u64 *)BL_SYS_INFO_DRAM_SIZE;
	struct pit_entry *ptn;
	bdev_t *dev;
	const char *name;
	unsigned int boot_dev;
	int len;
	const char *np;
	int noff;
	struct AvbOps *ops;
	bool unlock;
	struct boot_img_hdr *b_hdr = (boot_img_hdr *)BOOT_BASE;

	/* Get Secure DRAM information */
	soc_ver = exynos_smc(SMC_CMD_GET_SOC_INFO, SOC_INFO_TYPE_VERSION, 0, 0);
        if (soc_ver == SOC_INFO_VERSION(SOC_INFO_MAJOR_VERSION, SOC_INFO_MINOR_VERSION)) {
		sec_dram_base = exynos_smc(SMC_CMD_GET_SOC_INFO,
						SOC_INFO_TYPE_SEC_DRAM_BASE,
						0,
						0);
		if (sec_dram_base == (unsigned long)ERROR_INVALID_TYPE) {
			printf("get secure memory base addr error!!\n");
			while (1);
		}

		sec_dram_size = (unsigned int)exynos_smc(SMC_CMD_GET_SOC_INFO,
							SOC_INFO_TYPE_SEC_DRAM_SIZE,
							0,
							0);
		if (sec_dram_size == (unsigned int)ERROR_INVALID_TYPE) {
			printf("get secure memory size error!!\n");
			while (1);
		}
	} else {
		printf("[ERROR] el3_mon is old version. (0x%x)\n", soc_ver);
		while (1);
	}

	sec_dram_end = sec_dram_base + sec_dram_size;

	printf("SEC_DRAM_BASE[%#lx]\n", sec_dram_base);
	printf("SEC_DRAM_SIZE[%#x]\n", sec_dram_size);

	/* Get secure page table for DRM information */
	sec_pt_base = exynos_smc(SMC_DRM_GET_SOC_INFO,
					SOC_INFO_SEC_PGTBL_BASE,
					0,
					0);
	if (sec_pt_base == ERROR_DRM_INVALID_TYPE) {
		printf("[SEC_PGTBL_BASE] Invalid type\n");
		sec_pt_base = 0;
	} else if (sec_pt_base == ERROR_DRM_FW_INVALID_PARAM) {
		printf("[SEC_PGTBL_BASE] Do not support SMC for SMC_DRM_GET_SOC_INFO\n");
		sec_pt_base = 0;
	} else if (sec_pt_base == (unsigned long)ERROR_NO_DRM_FW_INITIALIZED) {
		printf("[SEC_PGTBL_BASE] DRM LDFW is not initialized\n");
		sec_pt_base = 0;
	} else if (sec_pt_base & MASK_1MB) {
		printf("[SEC_PGTBL_BASE] Not aligned with 1MB\n");
		sec_pt_base = 0;
	}

	sec_pt_size = (unsigned int)exynos_smc(SMC_DRM_GET_SOC_INFO,
						SOC_INFO_SEC_PGTBL_SIZE,
						0,
						0);
	if (sec_pt_size == ERROR_DRM_INVALID_TYPE) {
		printf("[SEC_PGTBL_SIZE] Invalid type\n");
		sec_pt_size = 0;
	} else if (sec_pt_size == ERROR_DRM_FW_INVALID_PARAM) {
		printf("[SEC_PGTBL_SIZE] Do not support SMC for SMC_DRM_GET_SOC_INFO\n");
		sec_pt_size = 0;
	} else if (sec_pt_size == (unsigned int)ERROR_NO_DRM_FW_INITIALIZED) {
		printf("[SEC_PGTBL_SIZE] DRM LDFW is not initialized\n");
		sec_pt_size = 0;
	} else if (sec_pt_base & MASK_1MB) {
		printf("[SEC_PGTBL_SIZE] Not aligned with 1MB\n");
		sec_pt_size = 0;
	}

	sec_pt_end = sec_pt_base + sec_pt_size;

	printf("SEC_PGTBL_BASE[%#lx]\n", sec_pt_base);
	printf("SEC_PGTBL_SIZE[%#x]\n", sec_pt_size);

	/*
	 * In this here, it is enabled cache. So you don't use blk read/write function
	 * If you modify dtb, you must use under set_bootargs function.
	 * And if you modify bootargs, you will modify in set_bootargs function.
	 */

	merge_dto_to_main_dtb();

	/* Secure memories are carved-out in case of EVT1 */
	/*
	 * 1st DRAM node
	 */
	add_dt_memory_node(DRAM_BASE,
				sec_dram_base - DRAM_BASE);
	/*
	 * 2nd DRAM node
	 */
	if (sec_pt_base && sec_pt_size) {
		add_dt_memory_node(sec_dram_end,
					sec_pt_base - sec_dram_end);
		add_dt_memory_node(sec_pt_end,
					(DRAM_BASE + SIZE_2GB)
					- sec_pt_end);
	} else {
		add_dt_memory_node(sec_dram_end,
					(DRAM_BASE + SIZE_2GB)
					- sec_dram_end);
	}

	/*
	 * 3rd DRAM node
	 */
	add_dt_memory_node(DRAM_BASE2, SIZE_2GB);
	if (dram_size == 0x180000000)
		add_dt_memory_node(0x900000000, SIZE_2GB);

	resize_dt(SZ_4K);
	set_usb_serialno();

	/* Disable CCI unit and isolate phy for USB */
	phy_usb_terminate();

	if (readl(EXYNOS9610_POWER_SYSIP_DAT0) == REBOOT_MODE_RECOVERY) {
		sprintf(str, "<0x%x>", RAMDISK_BASE);
		set_fdt_val("/chosen", "linux,initrd-start", str);

		sprintf(str, "<0x%x>", RAMDISK_BASE + b_hdr->ramdisk_size);
		set_fdt_val("/chosen", "linux,initrd-end", str);
	} else if (readl(EXYNOS9610_POWER_SYSIP_DAT0) == REBOOT_MODE_FACTORY) {
		noff = fdt_path_offset (fdt_dtb, "/chosen");
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s %s", np, "androidboot.mode=sfactory");
		fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
		printf("Enter samsung factory mode...");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "Enter samsung factory mode...");
	}

	sprintf(str, "<0x%x>", ECT_BASE);
	set_fdt_val("/ect", "parameter_address", str);

	sprintf(str, "<0x%x>", ECT_SIZE);
	set_fdt_val("/ect", "parameter_size", str);

	memset(buf, 0, sizeof(buf));
	update_boot_reason(buf);
	noff = fdt_path_offset(fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
	snprintf(str, BUFFER_SIZE, "%s androidboot.bootreason=%s", np, buf);
	fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);

	if (get_charger_mode() && readl(EXYNOS9610_POWER_SYSIP_DAT0) != REBOOT_MODE_FACTORY) {
		noff = fdt_path_offset (fdt_dtb, "/chosen");
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s %s", np, "androidboot.mode=charger");
		fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
		printf("Enter charger mode...");
	}

	/* Add booting slot */
	noff = fdt_path_offset(fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
	if (ab_current_slot())
		snprintf(str, BUFFER_SIZE, "%s %s", np, "androidboot.slot_suffix=_b");
	else
		snprintf(str, BUFFER_SIZE, "%s %s", np, "androidboot.slot_suffix=_a");
	fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);

	noff = fdt_path_offset(fdt_dtb, "/reserved-memory/modem_if");
	if (noff >= 0) {
		np = fdt_getprop(fdt_dtb, noff, "reg", &len);
		if (len >= 0) {
			memset(str, 0, BUFFER_SIZE);
			memcpy(str, np, len);

			boot_dev = get_boot_device();
			if (boot_dev == BOOT_UFS)
				name = "scsi0";
			else {
				printf("Boot device: 0x%x. Unsupported boot device!\n", boot_dev);
				return;
			}

			/* get modem partition info */
			dev = bio_open(name);
			if (ab_current_slot())
				ptn = pit_get_part_info("modem_b");
			else
				ptn = pit_get_part_info("modem_a");
			/* load modem header */
			dev->new_read(dev, (void *)(u64)be32_to_cpu(((const u32 *)str)[1]), ptn->blkstart, 16);
			bio_close(dev);
		}
	}

	if (!(readl(EXYNOS9610_POWER_SYSIP_DAT0) == REBOOT_MODE_RECOVERY)) {
		/* set AVB args */
		get_ops_addr(&ops);
		ops->read_is_device_unlocked(ops, &unlock);
		noff = fdt_path_offset (fdt_dtb, "/chosen");
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s %s %s", np, cmdline, verifiedbootstate);
		fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
	}

	if (readl(EXYNOS9610_POWER_SYSIP_DAT0) == REBOOT_MODE_RECOVERY) {
		/* Set bootargs for recovery mode */
		remove_string_from_bootargs("skip_initramfs ");
		remove_string_from_bootargs("ro init=/init ");

		noff = fdt_path_offset (fdt_dtb, "/chosen");
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s %s", np, "root=/dev/ram0");
		fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
	}

	noff = fdt_path_offset (fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
	printf("\nbootargs: %s\n", np);

	set_bootargs();
	resize_dt(0);
}

int cmd_scatter_load_boot(int argc, const cmd_args *argv);

int load_boot_images(void)
{
	struct pit_entry *ptn;
	cmd_args argv[6];

	if (ab_current_slot())
		ptn = pit_get_part_info("boot_b");
	else
		ptn = pit_get_part_info("boot_a");

	if (ptn == 0) {
		printf("Partition 'kernel' does not exist\n");
		return -1;
	} else {
		pit_access(ptn, PIT_OP_LOAD, (u64)BOOT_BASE, 0);
	}

	argv[1].u = BOOT_BASE;
	argv[2].u = KERNEL_BASE;
	argv[3].u = RAMDISK_BASE;
	argv[4].u = DT_BASE;
	argv[5].u = DTBO_BASE;
	cmd_scatter_load_boot(5, argv);

	return 0;
}

int cmd_boot(int argc, const cmd_args *argv)
{
	struct exynos_gpio_bank *bank = (struct exynos_gpio_bank *)EXYNOS9610_GPA1CON;
	int gpio = 5;	/* Volume Up */
	unsigned int val;

	fdt_dtb = (struct fdt_header *)DT_BASE;
	dtbo_table = (struct dt_table_header *)DTBO_BASE;
#if defined(CONFIG_USE_AVB20)
	int avb_ret = 0;
	uint32_t lock_state;
#endif
	int ab_ret = 0;

	val = exynos_gpio_get_value(bank, gpio);
	if (!val) {
		writel(REBOOT_MODE_FACTORY, EXYNOS9610_POWER_SYSIP_DAT0);
		printf("Pressed key combination to enter samsung factory mode!\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK,
			"Pressed key combination to enter samsung factory mode!");
	}

	ab_ret = ab_update_slot_info();
	if (ab_ret < 0) {
		printf("AB update error! Error code: %d\n", ab_ret);
		start_usb_gadget();
		do {
			asm volatile("wfi");
		} while(1);
	}

	load_boot_images();

#if defined(CONFIG_USE_AVB20)
	if (ab_current_slot())
		avb_ret = avb_main("_b", cmdline, verifiedbootstate);
	else
		avb_ret = avb_main("_a", cmdline, verifiedbootstate);
	printf("AVB: boot/dtbo image verification result: %d\n", avb_ret);

	rpmb_get_lock_state(&lock_state);
	printf("lock state: %d\n", lock_state);
	if(lock_state) {
		if (avb_ret) {
			printf("AVB failed! Resetting!\n");
			/* Delay for data write HW operation of ab_update_slot_info()
				on AB_SLOTINFO_PART partition. */
			mdelay(500);
			writel(0x1, EXYNOS9610_SWRESET);
			do {
				asm volatile("wfi");
			} while(1);
		}
	}
#endif

	configure_dtb();
	configure_ddi_id();

	/* Power Off Notification to storage */
	scsi_do_ssu();

	val = readl(EXYNOS9610_POWER_SYSIP_DAT0);
	if (val == REBOOT_MODE_RECOVERY || val == REBOOT_MODE_FACTORY) {
		writel(0, EXYNOS9610_POWER_SYSIP_DAT0);
	} else if (val == REBOOT_MODE_FASTBOOT) {
		writel(0, EXYNOS9610_POWER_SYSIP_DAT0);
		printf("Entering fastboot.\n");
		print_lcd_update(FONT_RED, FONT_BLACK, "Entering fastboot.");
		start_usb_gadget();
		return 0;
	}
	/* notify EL3 Monitor end of bootloader */
	exynos_smc(SMC_CMD_END_OF_BOOTLOADER, 0, 0, 0);

	/* before jumping to kernel. disble arch_timer */
	arm_generic_timer_disable();

	clean_invalidate_dcache_all();
	disable_mmu_dcache();

	void (*kernel_entry)(int r0, int r1, int r2, int r3);

	kernel_entry = (void (*)(int, int, int, int))KERNEL_BASE;
	kernel_entry(DT_BASE, 0, 0, 0);

	return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("boot", "start kernel booting", &cmd_boot)
STATIC_COMMAND_END(boot);
