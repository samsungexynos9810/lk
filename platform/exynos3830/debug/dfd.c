/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 *
 * No part of this software, either material or conceptual may be copied or
 * distributed, transmitted, transcribed, stored in a retrieval system or
 * translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed to third parties
 * without the express written permission of Samsung Electronics.
 *
 */
#include <reg.h>
#include <string.h>
#include <arch/ops.h>
#include <lib/font_display.h>
#include <platform/sizes.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/delay.h>
#include <platform/dfd.h>

#define TIMEOUT	100000

void wfi(void)
{
	asm volatile ("wfi");
}

static inline void pmu_set_bit_atomic(u32 offset, u32 bit)
{
        writel(bit, EXYNOS3830_POWER_BASE + (offset | 0xc000));
}

static inline void pmu_clr_bit_atomic(u32 offset, u32 bit)
{
        writel(bit, EXYNOS3830_POWER_BASE + (offset | 0x8000));
}

static u32 pmu_cpu_offset(u32 cpu)
{
	u32 tmp = 0x0;

	if (cpu >= BIG_CORE_START) {
		cpu -= 4;
		tmp = 0x400;
	}

	return tmp + (0x80 * cpu);
}

static int dfd_wait_complete(u32 cpu)
{
	u32 ret;
	u32 loop = 1000;

	do {
		ret = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
		if (ret & (1 << cpu))
			return 1;

		udelay(100);
	} while (loop--);

	return 0;
}

static void dfd_display_panic_reason(void)
{
	char *str = (char *)CONFIG_RAMDUMP_PANIC_REASON;
	int is_string = 0;
	int cnt = 0;

	for (cnt = 0; cnt < CONFIG_RAMDUMP_PANIC_LOGSZ; cnt++, str++)
		if (*str == 0x0)
			is_string = 1;

	if (!is_string) {
		str = (char *)CONFIG_RAMDUMP_PANIC_REASON;
		str[CONFIG_RAMDUMP_PANIC_LOGSZ - 1] = 0x0;
	}

	printf("%s\n", (char *)CONFIG_RAMDUMP_PANIC_REASON);
	print_lcd_update(FONT_YELLOW, FONT_RED, "%s", CONFIG_RAMDUMP_PANIC_REASON);
}

static int dfd_check_panic_stat(u32 cpu)
{
	u32 val = readl(CONFIG_RAMDUMP_CORE_PANIC_STAT + (cpu * REG_OFFSET));

	if (val == RAMDUMP_SIGN_PANIC)
		return 1;
	else if (val == RAMDUMP_SIGN_RESERVED)
		return 2;
	return 0;
}

#ifdef CONFIG_RAMDUMP_GPR
void dfd_display_reboot_reason(void)
{
	u32 ret;

	ret = readl(CONFIG_RAMDUMP_REASON);
	print_lcd_update(FONT_WHITE, FONT_BLACK, "reboot reason: ");

	switch (ret) {
	case RAMDUMP_SIGN_PANIC:
		printf("retboot reason: 0x%x - Kernel PANIC\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Kernel PANIC", ret);
		dfd_display_panic_reason();
		break;
	case RAMDUMP_SIGN_NORMAL_REBOOT:
		printf("retboot reason: 0x%x - User Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLACK, "0x%x - User Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_BL_REBOOT:
		printf("0x%x - BL Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLACK, "0x%x - BL Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_FORCE_REBOOT:
		printf("retboot reason: 0x%x - Forced Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLUE, "0x%x - Forced Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_SAFE_FAULT:
		printf("retboot reason: 0x%x - Safe Kernel PANIC\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Safe Kernel PANIC", ret);
		dfd_display_panic_reason();
		break;
	case RAMDUMP_SIGN_RESET:
	default:
		printf("retboot reason: 0x%x - Power/Emergency Reset\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Power/Emergency Reset", ret);
		break;
	}
}

void dfd_display_core_stat(void)
{
	int val;
	u32 ret;

	printf("Core stat at previous(KERNEL)\n");
	for (val = 0; val < NR_CPUS; val++) {
		ret = readl(CONFIG_RAMDUMP_CORE_POWER_STAT + (val * REG_OFFSET));
		printf("Core%d: ", val);
		switch (ret) {
		case RAMDUMP_SIGN_ALIVE:
			printf("Alive");
			break;
		case RAMDUMP_SIGN_DEAD:
			printf("Dead");
			break;
		case RAMDUMP_SIGN_RESET:
		default:
			printf("Power/Emergency Reset: 0x%x", ret);
			break;
		}

		ret = readl(CONFIG_RAMDUMP_CORE_PANIC_STAT + (val * REG_OFFSET));
		switch (ret) {
		case RAMDUMP_SIGN_PANIC:
			printf("/PANIC\n");
			break;
		case RAMDUMP_SIGN_RESET:
		case RAMDUMP_SIGN_RESERVED:
		default:
			printf("\n");
			break;
		}
	}
}

void dfd_set_dump_en_for_cacheop(int en)
{
	if (en)
		pmu_set_bit_atomic(RESET_SEQUENCER_OFFSET, DUMP_EN_BIT);
	else
		pmu_clr_bit_atomic(RESET_SEQUENCER_OFFSET, DUMP_EN_BIT);
}
#endif
static void dfd_get_gpr(u64 cpu)
{
	int i, idx;
	u64 *cpureg = (u64 *)(CONFIG_RAMDUMP_COREREG + (cpu * COREREG_OFFSET));
	u64 *iram = (u64 *)DUMPGPR_BASE;

	for (i = X0, idx = X0; i < GPR_END; ++i) {
		if (i == X19)
			idx = 19;
		cpureg[idx++] = iram[i];
	}
}

static void dfd_get_pc_value(void)
{
	u64 i;

	for (i = 0; i < NR_CPUS; i++) {
		u64 *cpu_reg = (u64 *)(CONFIG_RAMDUMP_COREREG + (i * COREREG_OFFSET));
		u64 iram = DUMPPC_BASE + (i * 0x10);

		cpu_reg[PC] = readq(iram);
		cpu_reg[POWER_STATE] = readl(iram + 0x8);

		if (((cpu_reg[PC] >> 40) & 0xff) == 0xff)
			cpu_reg[PC] |= 0xffffff0000000000;
		printf("Core%llu: reg : 0x%llX\n", i, cpu_reg[PC]);
	}
}

/*
 * FLUSH_SKIP : skip cache flush
 * FLUSH_LEVEL 1 : L1, L2 cache flush (local cache flush)
 * FLUSH_LEVEL 2 : L3 cache flush (cluster cache flush)
 * FLUSH_LEVEL 3 : L1, L2, L3 cache flush (all cache flush)
 */
static void dfd_set_cache_flush_level(void)
{
	int cl0_on = -1, cl1_on = -1;
	u64 cpu, val, stat;
	u64 *cpu_reg;

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		u32 offset = cpu * REG_OFFSET;
		cpu_reg = (u64 *)(CONFIG_RAMDUMP_COREREG + (cpu * COREREG_OFFSET));
		if (!cpu_reg[POWER_STATE]) {
			stat = FLUSH_SKIP;
		} else {
			stat = FLUSH_LEVEL1;
			if (cpu <= LITTLE_CORE_LAST)
				cl0_on = cpu;
			else
				cl1_on = cpu;
		}
		writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + offset);
		printf("Core%llu: Initial policy - Cache Flush Level %llu\n", cpu, stat);
	}

	/* conclude core which runs cache flush level 2 in little cluster */
	if (cl0_on <= 0) {
		/* core0 runs cache flush level 2 */
		val = FLUSH_LEVEL2;
		cl0_on = 0;
	} else {
		val = FLUSH_LEVEL3;
	}

	stat = val;
	writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + (cl0_on * REG_OFFSET));
	printf("Core%d: Cache Flush Level changed => %llu\n", cl0_on, stat);

	if (cl1_on >= 0) {
		stat = FLUSH_LEVEL3;
		writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + (cl1_on * REG_OFFSET));
		printf("Core%d: Cache Flush Level changed => %llu\n", cl1_on, stat);
	}
}

static void dfd_flush_dcache_level(u32 level, u32 invalidate)
{
	__arch_clean_and_invalidate_dcache_level(level, invalidate);
}

static void dfd_flush_dcache_all(void)
{
	arch_clean_and_invalidate_dcache_all();
}

void dfd_secondary_cpu_cache_flush(u32 cpu)
{
	u32 val;

	dfd_get_gpr(cpu);
	do {
		val = readl(CONFIG_RAMDUMP_WAKEUP_WAIT);
		if (val & (1 << cpu))
			break;
	} while (1);

	/* Get Cache Flush Level */
	val = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET));
	switch (val) {
	case 0:
		break;
	case 1:
		/* FLUSH_LEVEL1 */
		dfd_flush_dcache_level(val - 1, 0);
		break;
	case 2:
		/* FLUSH_LEVEL2 */
		dfd_flush_dcache_level(val - 1, 0);
		break;
	case 3:
		/* FLUSH_LEVEL3 (all cache flush) */
		dfd_flush_dcache_all();
		break;
	default:
		break;
	}
	/* Write own bit to inform finishing dumpGPR */
	val = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
	writel((val | (1 << cpu)), CONFIG_RAMDUMP_DUMP_GPR_WAIT);
off:
	if (cpu != 0) {
		//cpu_boot(CPU_OFF_PSCI_ID, 0, 0);
		do {
		//	wfi();
		} while (1);
	}
}

static void dfd_clear_reset_disable(void)
{
	for (int cpu = 0; cpu < NR_CPUS; cpu++)
		pmu_clr_bit_atomic(PMU_CPU_OUT_OFFSET + pmu_cpu_offset(cpu), CPU_CLR_DBGL1RSTDIS);

	pmu_clr_bit_atomic(PMU_CL0_NCPU_OUT_OFFSET, NCPU_CLR_DBGL3RSTDIS);
	pmu_clr_bit_atomic(PMU_CL1_NCPU_OUT_OFFSET, NCPU_CLR_DBGL3RSTDIS);
}

#ifdef CONFIG_RAMDUMP_GPR
void dfd_run_post_processing(void)
{
	u32 cpu_logical_map[NR_CPUS] = {
		CPU0_LOGICAL_MAP,
		CPU1_LOGICAL_MAP,
		CPU2_LOGICAL_MAP,
		CPU3_LOGICAL_MAP,
		CPU4_LOGICAL_MAP,
		CPU5_LOGICAL_MAP,
		CPU6_LOGICAL_MAP,
		CPU7_LOGICAL_MAP
	};
	u32 cpu, val, cpu_mask = 0;
	int ret;

	if (!(readl(EXYNOS3830_POWER_BASE + RESET_SEQUENCER_OFFSET) & DUMP_EN)) {
		printf("DUMP_EN disabled. Skip debug info.\n");
		return;
	}

	dfd_set_dump_en_for_cacheop(0);
	dfd_clear_reset_disable();

	printf("---------------------------------------------------------\n");
	printf("Watchdog or Warm Reset Detected.\n");

	printf("Display PC value\n");
	dfd_get_pc_value();
	dfd_get_gpr(0);

	//llc_flush_disable();

	dfd_set_cache_flush_level();
	//Wake up secondary CPUs.
	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		val = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET));
		if (val != FLUSH_SKIP)
			cpu_mask |= (1 << cpu);

		if (cpu == 0)
			continue;

		ret = cpu_boot(CPU_ON_PSCI_ID, cpu_logical_map[cpu], (u64)dfd_entry_point);
		if (ret) {
			printf("Core%d: ERR failed power on: 0x%x\n", cpu, ret);
			ret = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
			writel((ret | 1 << cpu), CONFIG_RAMDUMP_DUMP_GPR_WAIT);
			continue;
		}
		mdelay(100);
	}

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		val = readl(CONFIG_RAMDUMP_WAKEUP_WAIT);
		writel(val | (1 << cpu), CONFIG_RAMDUMP_WAKEUP_WAIT);
		if (cpu == 0)
			dfd_secondary_cpu_cache_flush(cpu);

		if (!dfd_wait_complete(cpu)) {
			printf("Core%d: ERR wait timeout.\n", cpu);
			continue;
		}

		printf("Core%d: finished Cache Flush level:%d (0x%x)\n", cpu,
		(readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET))),
		readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT));
	}
	printf("---------------------------------------------------------\n");
}
#endif

unsigned int clear_llc_init_state(void)
{
	pmu_clr_bit_atomic(RESET_SEQUENCER_OFFSET, LLC_INIT_BIT);

	return readl(EXYNOS3830_POWER_BASE + RESET_SEQUENCER_OFFSET);
}

void reset_prepare_board(void)
{
	dfd_set_dump_en_for_cacheop(0);
	/* Clear debug level when reset */
	writel(0, CONFIG_RAMDUMP_DEBUG_LEVEL);
}

const char *debug_level_val[] = {
	"low",
	"mid",
};

void set_debug_level(const char *buf)
{
	int i, debug_level = DEBUG_LEVEL_MID;

	if (!buf)
		return;

	for (i = 0; i < (int)ARRAY_SIZE(debug_level_val); i++) {
		if (!strncmp(buf, debug_level_val[i],
			strlen(debug_level_val[i]))) {
			debug_level = i;
			goto debug_level_print;
		}
	}

debug_level_print:
	/* Update debug_level to reserved region */
	writel(debug_level | DEBUG_LEVEL_PREFIX, CONFIG_RAMDUMP_DEBUG_LEVEL);
	printf("debug level: %s\n", debug_level_val[debug_level]);
}

void set_debug_level_by_env(void)
{
#if 0
	char buf[16] = {0, };
	int ret;

	ret = getenv_s("debug_level", buf, sizeof(buf));
	if (ret > 0) {
		printf("Change ");
		set_debug_level(buf);
	}
#endif
}
