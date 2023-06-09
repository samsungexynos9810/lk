LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := ${LOCAL_DIR}

WITH_SMP := 0

MODULE_SRCS += \
    $(LOCAL_DIR)/platform.c \
	$(LOCAL_DIR)/debug.c \
	$(LOCAL_DIR)/uart_simple.c \
	$(LOCAL_DIR)/delay.c \
	$(LOCAL_DIR)/ufs.c \
	$(LOCAL_DIR)/ufs-cal-9630.c \
	$(LOCAL_DIR)/usb.c \
	$(LOCAL_DIR)/fastboot/decompress_ext4.c \
	$(LOCAL_DIR)/boot/cmd_scatter_load_boot.c \
	$(LOCAL_DIR)/boot/cmd_boot.c \
	$(LOCAL_DIR)/boot/cmd_display.c \
	$(LOCAL_DIR)/security/smc_aarch64.c \
	$(LOCAL_DIR)/security/ldfw.c \
	$(LOCAL_DIR)/security/avb_ops.c \
	$(LOCAL_DIR)/security/avb_main.c \
	$(LOCAL_DIR)/security/sb_api.c \
	$(LOCAL_DIR)/security/cm_api.c \
	$(LOCAL_DIR)/security/otp_v20.c \
	$(LOCAL_DIR)/speedy/speedy.c \
	$(LOCAL_DIR)/pmic/pmic_s2mpu10_11.c \
	$(LOCAL_DIR)/pmic/if_pmic_s2mu107.c \
	$(LOCAL_DIR)/pmic/fg_s2mu107.c \
	$(LOCAL_DIR)/ab_update/ab_update.c \
	$(LOCAL_DIR)/gpio_init.S \
	$(LOCAL_DIR)/gpio.c \
	$(LOCAL_DIR)/dfd.c \
	$(LOCAL_DIR)/dfd_verify.c \
	$(LOCAL_DIR)/dfd_asm_helper.S \
	$(LOCAL_DIR)/dpu_cal/decon_reg.c \
	$(LOCAL_DIR)/dpu_cal/dpp_reg.c \
	$(LOCAL_DIR)/dpu_cal/dsim_reg.c \
	$(LOCAL_DIR)/debug-snapshot-uboot.c \
	$(LOCAL_DIR)/wdt/wdt_recovery.c \
	$(LOCAL_DIR)/fdt.c \
	$(LOCAL_DIR)/dram_training/dram_training.c \
	$(LOCAL_DIR)/mmu/cpu_a.S \
	$(LOCAL_DIR)/mmu/mmu.c \
	$(LOCAL_DIR)/debug-store-ramdump.c \
	$(LOCAL_DIR)/power.c \
	$(LOCAL_DIR)/tmu.c \
	$(LOCAL_DIR)/flexpmu_dbg.c

MODULE_DEPS += \
	dev/interrupt/arm_gic \
	dev/timer/arm_generic \
	dev/scsi \
	lib/cksum \
	dev/usb/dwc3 \
	dev/usb/phy/exynos \
	dev/usb/device/fastboot 

LINKER_SCRIPT += $(BUILDDIR)/system-onesegment.ld

include make/module.mk
