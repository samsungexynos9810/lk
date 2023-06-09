/* Copyright (c) 2018 Samsung Electronics Co, Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * Copyright@ Samsung Electronics Co. LTD
 * Manseok Kim <manseoks.kim@samsung.com>

 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _REGS_DISP_SS_H
#define _REGS_DISP_SS_H

#define DISP_DPU_MIPI_PHY_CON			0x1008
/* _v : [0,1] */
#define SEL_RESET_DPHY_MASK(_v)			(0x1 << (4 + (_v)))
#define M_RESETN_M4S4_TOP_MASK			(0x1 << 0)

#define DISP_DPU_TE_QACTIVE_PLL_EN		0x1010
#define TE_QACTIVE_PLL_EN			(0x1 << 0)

#endif /* _REGS_DISP_SS_H */

#ifndef _REGS_DECON_H
#define _REGS_DECON_H

/*
 * [ BLK_DPU BASE ADDRESS ]
 *
 * - CMU_DPU			0x1300_0000
 * - DTZPC_DPU			0x1301_0000
 * - SYSREG_DPU			0x1302_0000
 * - DECON0_SECURE		0x1304_0000
 * - DPP				0x1305_0000
 * - DECON0				0x1306_0000
 * - MIPI_DSIM0			0x1307_0000
 * - MIPI_DPHY			0x1308_0000
 * - DPU_DMA_SECURE		0x130A_0000
 * - DPU_DMA			0x130B_0000
 * - SYSMMU_DPU_NS		0x130C_0000
 * - SYSMMU_DPU_S		0x130D_0000
 * - PPMU_DPU			0x130E_0000
 */

/*
 *	IP			start_offset	end_offset
 *=================================================
 *	DECON			0x0000		0x0fff
 *	BLENDER			0x1000		0x1fff
 *-------------------------------------------------
 *	SHD_DECON		0x7000		0x7FFF
 *-------------------------------------------------
 *	SHD_BLENDER		0x8000		0x8FFF
 *-------------------------------------------------
 */


/*
 * DECON registers
 * ->
 * updated by SHADOW_REG_UPDATE_REQ[31] : SHADOW_REG_UPDATE_REQ
 *	(0x0000~0x011C, 0x0230~0x209C )
 */

#define GLOBAL_CONTROL				0x0000
#define GLOBAL_CONTROL_SRESET			(1 << 28)
#define GLOBAL_CONTROL_OPERATION_MODE_F		(1 << 8)
#define GLOBAL_CONTROL_OPERATION_MODE_VIDEO_F	(0 << 8)
#define GLOBAL_CONTROL_OPERATION_MODE_CMD_F	(1 << 8)
#define GLOBAL_CONTROL_IDLE_STATUS		(1 << 5)
#define GLOBAL_CONTROL_RUN_STATUS		(1 << 4)
#define GLOBAL_CONTROL_DECON_EN			(1 << 1)
#define GLOBAL_CONTROL_DECON_EN_F		(1 << 0)

#define SRAM_SHARE_ENABLE_MAIN			0x0030
#define FF0_SRAM_SHARE_ENABLE_F			(1 << 12)
#define ALL_SRAM_SHARE_ENABLE			(0x1000 << 0)
#define ALL_SRAM_SHARE_DISABLE			(0x0)

#define INTERRUPT_ENABLE			0x0040
#define DPU_FRAME_DONE_INT_EN			(1 << 13)
#define DPU_FRAME_START_INT_EN			(1 << 12)
#define DPU_EXTRA_INT_EN			(1 << 4)
#define DPU_INT_EN				(1 << 0)
#define INTERRUPT_ENABLE_MASK			0x3011

#define EXTRA_INTERRUPT_ENABLE			0x0044
/* These are dummny to prevent compile error */
#define DPU_RESOURCE_CONFLICT_INT_EN		(1 << 8)
/* These are dummny to prevent compile error */
#define DPU_TIME_OUT_INT_EN			(1 << 4)
#define DPU_ERROR_INT_EN			(1 << 0)

#define TIME_OUT_VALUE				0x0048

#define INTERRUPT_PENDING			0x004C
#define DPU_FRAME_DONE_INT_PEND			(1 << 13)
#define DPU_FRAME_START_INT_PEND		(1 << 12)
#define DPU_EXTRA_INT_PEND			(1 << 4)

#define EXTRA_INTERRUPT_PENDING			0x0050
/* These are dummny to prevent compile error */
#define DPU_RESOURCE_CONFLICT_INT_PEND		(1 << 8)
/* These are dummny to prevent compile error */
#define DPU_TIME_OUT_INT_PEND			(1 << 4)
#define DPU_ERROR_INT_PEND				(1 << 0)

#define SHADOW_REG_UPDATE_REQ			0x0060
#define SHADOW_REG_UPDATE_REQ_GLOBAL		(1 << 31)
#define SHADOW_REG_UPDATE_REQ_WIN(_win)		(1 << (_win))
#define SHADOW_REG_UPDATE_REQ_FOR_DECON		(0xf)

#define HW_SW_TRIG_CONTROL			0x0070
#define HW_TRIG_SKIP(_v)			((_v) << 16)
#define HW_TRIG_SKIP_MASK			(0xff << 16)
#define HW_TRIG_ACTIVE_VALUE			(1 << 9)
#define SW_TRIG_EN				(1 << 8)
#define HW_TRIG_EDGE_POLARITY			(1 << 7)
#define HW_TRIG_MASK_DECON			(1 << 4)
#define HW_SW_TRIG_TIMER_CLEAR			(1 << 3)
#define HW_SW_TRIG_TIMER_EN			(1 << 2)
#define HW_TRIG_EN				(1 << 0)

#define HW_SW_TRIG_TIMER			0x0074

#define CLOCK_CONTROL_0				0x00F0
/*
 * [24] QACTIVE_VALUE = 0
 *   0: QACTIVE is dynamically changed by DECON h/w,
 *   1: QACTIVE is stuck to 1'b1
 * [22][20][16][12][0] AUTO_CG_EN_xxx
 */
/* clock gating is disabled on bringup */
#define CLOCK_CONTROL_0_CG_MASK			(0x511001 << 0)
#define CLOCK_CONTROL_0_QACTIVE_MASK		(0x1 << 24)

#define FRAME_FIFO_0_SIZE_CONTROL_0			0x0120
#define FRAME_FIFO_HEIGHT_F(_v)			((_v) << 16)
#define FRAME_FIFO_HEIGHT_MASK			(0x3fff << 16)
#define FRAME_FIFO_HEIGHT_GET(_v)			(((_v) >> 16) & 0x3fff)
#define FRAME_FIFO_WIDTH_F(_v)			((_v) << 0)
#define FRAME_FIFO_WIDTH_MASK			(0x3fff << 0)
#define FRAME_FIFO_WIDTH_GET(_v)			(((_v) >> 0) & 0x3fff)

#define FRAME_FIFO_0_SIZE_CONTROL_1			0x0124

#define FRAME_FIFO_TH_CONTROL_0			0x0130
#define FRAME_FIFO_0_TH_F(_v)           ((_v) << 0)
#define FRAME_FIFO_0_TH_MASK            (0xffff << 0)
#define FRAME_FIFO_0_TH_GET(_v)     (((_v) >> 0) & 0xffff)

#define FRAME_FIFO_0_LEVEL              0x0134

#define FORMATTER_CONTROL               0x0190
#define FORMATTER_PIXEL0123_ORDER_SWAP_F    (1 << 16)
#define FORMATTER_PIXEL23_ORDER_SWAP_F  (1 << 12)
#define FORMATTER_PIXEL01_ORDER_SWAP_F  (1 << 8)
#define FORMATTER_PIXEL_ORDER_SWAP(_a, _b, _c)  (_a << 16) | (_b << 12) | (_c << 8)
#define FORMATTER_PIXEL_ORDER_SWAP_MASK ((1 << 16) | (1 << 12) | (1 << 8))
#define FORMATTER_OUT_RGB_ORDER_F(_v)   ((_v) << 4)
#define FORMATTER_OUT_RGB_ORDER_MASK        (0x7 << 4)

#define BLENDER_BG_IMAGE_SIZE_0			0x0200
#define BLENDER_BG_HEIGHT_F(_v)			((_v) << 16)
#define BLENDER_BG_HEIGHT_MASK			(0x3fff << 16)
#define BLENDER_BG_HEIGHT_GET(_v)		(((_v) >> 16) & 0x3fff)
#define BLENDER_BG_WIDTH_F(_v)			((_v) << 0)
#define BLENDER_BG_WIDTH_MASK			(0x3fff << 0)
#define BLENDER_BG_WIDTH_GET(_v)		(((_v) >> 0) & 0x3fff)

#define BLENDER_BG_IMAGE_SIZE_1         0x0204

#define BLENDER_BG_IMAGE_COLOR_0		0x0208
#define BLENDER_BG_A_F(_v)			((_v) << 16)
#define BLENDER_BG_A_MASK			(0xff << 16)
#define BLENDER_BG_A_GET(_v)			(((_v) >> 16) & 0xff)
#define BLENDER_BG_R_F(_v)			((_v) << 0)
#define BLENDER_BG_R_MASK			(0x3ff << 0)
#define BLENDER_BG_R_GET(_v)			(((_v) >> 0) & 0x3ff)

#define BLENDER_BG_IMAGE_COLOR_1		0x020C
#define BLENDER_BG_G_F(_v)			((_v) << 16)
#define BLENDER_BG_G_MASK			(0x3ff << 16)
#define BLENDER_BG_G_GET(_v)			(((_v) >> 16) & 0x3ff)
#define BLENDER_BG_B_F(_v)			((_v) << 0)
#define BLENDER_BG_B_MASK			(0x3ff << 0)
#define BLENDER_BG_B_GET(_v)			(((_v) >> 0) & 0x3ff)

#define LRMERGER_MODE_CONTROL			0x0210
#define LRM23_MODE_F(_v)			((_v) << 16)
#define LRM23_MODE_MASK				(0x7 << 16)
#define LRM01_MODE_F(_v)			((_v) << 0)
#define LRM01_MODE_MASK				(0x7 << 0)

#define DATA_PATH_CONTROL_0			0x0214
#define WIN_MAPCOLOR_EN_F(_win)			(1 << (4*_win + 1))
#define WIN_EN_F(_win)				(1 << (4*_win + 0))

#define DATA_PATH_CONTROL_1			0x0218
#define WIN_CHMAP_F(_win, _ch)			(((_ch) & 0x7) << (4*_win))
#define WIN_CHMAP_MASK(_win)			(0x7 << (4*_win))

#define DATA_PATH_CONTROL_2			0x0230
#define ENHANCE_LOGIC_PATH_F(_v)        ((_v) << 12)
#define ENHANCE_LOGIC_PATH_MASK     (0x1 << 12)
#define ENHANCE_LOGIC_PATH_GET(_v)      (((_v) >> 12) & 0x1)
#define COMP_LINKIF_WB_PATH_F(_v)       ((_v) << 0)
#define COMP_LINKIF_WB_PATH_MASK        (0x1ff << 0)
#define COMP_LINKIF_WB_PATH_GET(_v)     (((_v) >> 0) & 0x1ff)

/* DECON CRC for ASB */
#define CRC_DATA_0				0x0280
#define CRC_DATA_DSIMIF1_GET(_v)		(((_v) >> 16) & 0xffff)
#define CRC_DATA_DSIMIF0_GET(_v)		(((_v) >> 0) & 0xffff)

#define CRC_DATA_2				0x0288
#define CRC_DATA_DP1_GET(_v)			(((_v) >> 16) & 0xffff)
#define CRC_DATA_DP0_GET(_v)			(((_v) >> 0) & 0xffff)

#define CRC_CONTROL				0x028C
#define CRC_COLOR_SEL(_v)			((_v) << 16)
#define CRC_COLOR_SEL_MASK			(0x3 << 16)
#define CRC_START				(1 << 0)

#define FRAME_COUNT				0x02A0

/* BLENDER */
#define WIN_CONTROL_0(_win)			(0x1000 + ((_win) * 0x30))
#define WIN_ALPHA1_F(_v)			(((_v) & 0xFF) << 24)
#define WIN_ALPHA1_MASK				(0xFF << 24)
#define WIN_ALPHA0_F(_v)			(((_v) & 0xFF) << 16)
#define WIN_ALPHA0_MASK				(0xFF << 16)
#define WIN_ALPHA_GET(_v, _n)			(((_v) >> (16 + 8 * (_n))) & 0xFF)
#define WIN_FUNC_F(_v)				(((_v) & 0xF) << 8)
#define WIN_FUNC_MASK				(0xF << 8)
#define WIN_FUNC_GET(_v)			(((_v) >> 8) & 0xf)
#define WIN_SRESET				(1 << 4)
#define WIN_ALPHA_MULT_SRC_SEL_F(_v)		(((_v) & 0x3) << 0)
#define WIN_ALPHA_MULT_SRC_SEL_MASK		(0x3 << 0)

#define WIN_CONTROL_1(_win)			(0x1004 + ((_win) * 0x30))
#define WIN_FG_ALPHA_D_SEL_F(_v)		(((_v) & 0xF) << 24)
#define WIN_FG_ALPHA_D_SEL_MASK			(0xF << 24)
#define WIN_BG_ALPHA_D_SEL_F(_v)		(((_v) & 0xF) << 16)
#define WIN_BG_ALPHA_D_SEL_MASK			(0xF << 16)
#define WIN_FG_ALPHA_A_SEL_F(_v)		(((_v) & 0xF) << 8)
#define WIN_FG_ALPHA_A_SEL_MASK			(0xF << 8)
#define WIN_BG_ALPHA_A_SEL_F(_v)		(((_v) & 0xF) << 0)
#define WIN_BG_ALPHA_A_SEL_MASK			(0xF << 0)

#define WIN_START_POSITION(_win)		(0x1008 + ((_win) * 0x30))
#define WIN_STRPTR_Y_F(_v)			(((_v) & 0x3FFF) << 16)
#define WIN_STRPTR_X_F(_v)			(((_v) & 0x3FFF) << 0)

#define WIN_END_POSITION(_win)			(0x100C + ((_win) * 0x30))
#define WIN_ENDPTR_Y_F(_v)			(((_v) & 0x3FFF) << 16)
#define WIN_ENDPTR_X_F(_v)			(((_v) & 0x3FFF) << 0)

#define WIN_COLORMAP_0(_win)			(0x1010 + ((_win) * 0x30))
#define WIN_MAPCOLOR_A_F(_v)			((_v) << 16)
#define WIN_MAPCOLOR_A_MASK			(0xff << 16)
#define WIN_MAPCOLOR_R_F(_v)			((_v) << 0)
#define WIN_MAPCOLOR_R_MASK			(0x3ff << 0)

#define WIN_COLORMAP_1(_win)			(0x1014 + ((_win) * 0x30))
#define WIN_MAPCOLOR_G_F(_v)			((_v) << 16)
#define WIN_MAPCOLOR_G_MASK			(0x3ff << 16)
#define WIN_MAPCOLOR_B_F(_v)			((_v) << 0)
#define WIN_MAPCOLOR_B_MASK			(0x3ff << 0)

#define WIN_START_TIME_CONTROL(_win)		(0x1018 + ((_win) * 0x30))
#define WIN_START_TIME_CONTROL_F(_v)		((_v) << 0)
#define WIN_START_TIME_CONTROL_MASK		(0x3fff << 0)

#define SHADOW_OFFSET				0x7000

/* These are dummny to prevent compile error */
#define DSC0_OFFSET				0x4000
#define DSC1_OFFSET				0x5000
#define DSC2_OFFSET				0x6000
/* These are dummny to prevent compile error */

#endif /* _REGS_DECON_H */
