/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _EXYNOS_USB_H
#define _EXYNOS_USB_H

/*
 * SYSREG
 */
#define EXYNOS_POWER_BASE		0x11860000
//#define USB_CFG_OFFSET			0x230
//#define USB_CFG_REG			(EXYNOS_SYSREG_BASE + USB_CFG_OFFSET)


#define USBDEVICE3_LINK_CH0_BASE	0x13200000
#define USBDEVICE3_PHYCTRL_CH0_BASE	0x131D0000

/* PHY CONTROL */
#define USB2_PHY_CONTROL_OFFSET	0x704
#endif

#define USB2_PHY_CONTROL			(EXYNOS_POWER_BASE+USB2_PHY_CONTROL_OFFSET)
//==========================
// Global Registers (Gxxxx)
//==========================
// Global Common Registers
#define rGSBUSCFG0			(USBDEVICE3_LINK_BASE + 0xc100)
#define rGSBUSCFG1			(USBDEVICE3_LINK_BASE + 0xc104)
#define rGTXTHRCFG			(USBDEVICE3_LINK_BASE + 0xc108)
#define rGRXTHRCFG			(USBDEVICE3_LINK_BASE + 0xc10c)
#define rGCTL				(USBDEVICE3_LINK_BASE + 0xc110)
#define rGEVTEN				(USBDEVICE3_LINK_BASE + 0xc114)
#define rGSTS				(USBDEVICE3_LINK_BASE + 0xc118)
#define rGSNPSID			(USBDEVICE3_LINK_BASE + 0xc120)
#define rGGPIO				(USBDEVICE3_LINK_BASE + 0xc124)
#define rGUID				(USBDEVICE3_LINK_BASE + 0xc128)
#define rGUCTL				(USBDEVICE3_LINK_BASE + 0xc12c)
#define rGBUSERRADDR_LO			(USBDEVICE3_LINK_BASE + 0xc130)
#define rGBUSERRADDR_HI			(USBDEVICE3_LINK_BASE + 0xc134)

// Global Port to USB Instance Mapping Registers
#define rGPRTBIMAP_LO			(USBDEVICE3_LINK_BASE + 0xc138)
#define rGPRTBIMAP_HI			(USBDEVICE3_LINK_BASE + 0xc13c)
#define rGPRTBIMAP_HS_LO		(USBDEVICE3_LINK_BASE + 0xc180)
#define rGPRTBIMAP_HS_HI		(USBDEVICE3_LINK_BASE + 0xc184)
#define rGPRTBIMAP_FS_LO		(USBDEVICE3_LINK_BASE + 0xc188)
#define rGPRTBIMAP_FS_HI		(USBDEVICE3_LINK_BASE + 0xc18c)

// Global Hardware Parameter Registers
#define rGHWPARAMS0			(USBDEVICE3_LINK_BASE + 0xc140)	// 0x20204000 @c510
#define rGHWPARAMS1			(USBDEVICE3_LINK_BASE + 0xc144)	// 0x0060c93b @c510
#define rGHWPARAMS2			(USBDEVICE3_LINK_BASE + 0xc148)	// 0x12345678 @c510
#define rGHWPARAMS3			(USBDEVICE3_LINK_BASE + 0xc14c)	// 0x10420085 @c510
#define rGHWPARAMS4			(USBDEVICE3_LINK_BASE + 0xc150)	// 0x48820004 @c510
#define rGHWPARAMS5			(USBDEVICE3_LINK_BASE + 0xc154)	// 0x04204108 @c510
#define rGHWPARAMS6			(USBDEVICE3_LINK_BASE + 0xc158)	// 0x04008020 @c510
#define rGHWPARAMS7			(USBDEVICE3_LINK_BASE + 0xc15c)	// 0x018516fe @c510
#define rGHWPARAMS8			(USBDEVICE3_LINK_BASE + 0xc600)	// 0x00000386 @c510

// Global Debug Registers
#define rGDBGFIFOSPACE			(USBDEVICE3_LINK_BASE + 0xc160)
#define rGDBGLTSSM			(USBDEVICE3_LINK_BASE + 0xc164)
#define rGDBGLSPMUX			(USBDEVICE3_LINK_BASE + 0xc170)
#define rGDBGLSP			(USBDEVICE3_LINK_BASE + 0xc174)
#define rGDBGEPINFO0			(USBDEVICE3_LINK_BASE + 0xc178)
#define rGDBGEPINFO1			(USBDEVICE3_LINK_BASE + 0xc17c)

// Global PHY Registers
#define rGUSB2PHYCFG			(USBDEVICE3_LINK_BASE + 0xc200)
#define rGUSB2I2CCTL			(USBDEVICE3_LINK_BASE + 0xc240)
#define rGUSB2PHYACC			(USBDEVICE3_LINK_BASE + 0xc280)
#define rGUSB3PIPECTL			(USBDEVICE3_LINK_BASE + 0xc2c0)

// Global FIFO Size Registers (0 <= num <= 15 @510)
#define rGTXFIFOSIZ(num)		((USBDEVICE3_LINK_BASE + 0xc300) + 0x04*num)
#define rGRXFIFOSIZ0			(USBDEVICE3_LINK_BASE + 0xc380)

// Global Event Buffer Registers (DWC_USB3_DEVICE_NUM_INT = 1 @C510, GHWPARAMS1[20:15])
#define rGEVNTADR_LO0			(USBDEVICE3_LINK_BASE + 0xc400)
#define rGEVNTADR_HI0			(USBDEVICE3_LINK_BASE + 0xc404)
#define rGEVNTSIZ0			(USBDEVICE3_LINK_BASE + 0xc408)
#define rGEVNTCOUNT0			(USBDEVICE3_LINK_BASE + 0xc40c)
#define rGFLADJ			(USBDEVICE3_LINK_BASE + 0xc630)

//==========================
// Device Registers (Dxxxx)
//==========================
// Device Common Registers
#define rDCFG				(USBDEVICE3_LINK_BASE + 0xc700)
#define rDCTL				(USBDEVICE3_LINK_BASE + 0xc704)
#define rDEVTEN				(USBDEVICE3_LINK_BASE + 0xc708)
#define rDSTS				(USBDEVICE3_LINK_BASE + 0xc70c)
#define rDGCMDPAR			(USBDEVICE3_LINK_BASE + 0xc710)
#define rDGCMD				(USBDEVICE3_LINK_BASE + 0xc714)
#define rDALEPENA			(USBDEVICE3_LINK_BASE + 0xc720)

// Device Endpoint Registers (0 <= ep <= 15)
#define rDOEPCMDPAR2(ep)		((USBDEVICE3_LINK_BASE + 0xc800) + 0x20*ep)
#define rDOEPCMDPAR1(ep)		((USBDEVICE3_LINK_BASE + 0xc804) + 0x20*ep)
#define rDOEPCMDPAR0(ep)		((USBDEVICE3_LINK_BASE + 0xc808) + 0x20*ep)
#define rDOEPCMD(ep)			((USBDEVICE3_LINK_BASE + 0xc80c) + 0x20*ep)

#define rDIEPCMDPAR2(ep)		((USBDEVICE3_LINK_BASE + 0xc810) + 0x20*ep)
#define rDIEPCMDPAR1(ep)		((USBDEVICE3_LINK_BASE + 0xc814) + 0x20*ep)
#define rDIEPCMDPAR0(ep)		((USBDEVICE3_LINK_BASE + 0xc818) + 0x20*ep)
#define rDIEPCMD(ep)			((USBDEVICE3_LINK_BASE + 0xc81c) + 0x20*ep)

//==========================
// USB DEVICE PHY CONTROL REGISTERS
//==========================
#define EXYNOS_PHY_LINKSYSTEM		(USBDEVICE3_PHYCTRL_BASE + 0x04)
#define EXYNOS_PHY_UTMI			(USBDEVICE3_PHYCTRL_BASE + 0x08)
#define EXYNOS_PHY_PIPE			(USBDEVICE3_PHYCTRL_BASE + 0x0C)
#define EXYNOS_PHY_CLKPWR		(USBDEVICE3_PHYCTRL_BASE + 0x10)
#define EXYNOS_PHY_REG0			(USBDEVICE3_PHYCTRL_BASE + 0x14)
#define EXYNOS_PHY_REG1			(USBDEVICE3_PHYCTRL_BASE + 0x18)
#define EXYNOS_PHY_PARAM0		(USBDEVICE3_PHYCTRL_BASE + 0x1C)
#define EXYNOS_PHY_PARAM1		(USBDEVICE3_PHYCTRL_BASE + 0x20)
#define EXYNOS_PHY_TERM			(USBDEVICE3_PHYCTRL_BASE + 0x24)
#define EXYNOS_PHY_TEST			(USBDEVICE3_PHYCTRL_BASE + 0x28)
#define EXYNOS_PHY_ADP			(USBDEVICE3_PHYCTRL_BASE + 0x2C)
#define EXYNOS_PHY_BATCHG		(USBDEVICE3_PHYCTRL_BASE + 0x30)
#define EXYNOS_PHY_RESUME		(USBDEVICE3_PHYCTRL_BASE + 0x34)
#define EXYNOS_PHY_LINK_PORT		(USBDEVICE3_PHYCTRL_BASE + 0x44)
#if defined(CONFIG_CPU_EXYNOS7870) || defined(CONFIG_CPU_EXYNOS7880) || defined(CONFIG_CPU_EXYNOS7570) || defined(CONFIG_CPU_EXYNOS7872) || defined(CONFIG_CPU_EXYNOS7885) \
		|| defined(CONFIG_CPU_EXYNOS9610) || defined(CONFIG_CPU_EXYNOS9110)
#define EXYNOS_PHY_HOSTPHYCTRL0	(USBDEVICE3_PHYCTRL_BASE + 0x54)
#define EXYNOS_PHY_HSPHYPLLTUNE	(USBDEVICE3_PHYCTRL_BASE + 0x70)
#endif

//========================================
// USB3.0 DRD LINK CONTROLLER REVISION
//========================================
#define EXYNOS_DRD_REVISION_250A	0x5533250a

