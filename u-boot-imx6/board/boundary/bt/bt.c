/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <malloc.h>
#include <asm/arch/mx6-pins.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/fbpanel.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/sata.h>
#include <asm/imx-common/spi.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mxc_hdmi.h>
#include <i2c.h>
#include <input.h>
#include <netdev.h>
#include <splash.h>
#include <usb/ehci-fsl.h>

DECLARE_GLOBAL_DATA_PTR;

#define AUD_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define BUTTON_PAD_CTRL (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define CSI_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define OUTPUT_40OHM (PAD_CTL_SPEED_MED|PAD_CTL_DSE_40ohm)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm     | PAD_CTL_SRE_FAST)

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define WEAK_PULLDN	(PAD_CTL_PUS_100K_DOWN |		\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

#define WEAK_PULLUP	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

/*
 *
 */
static const iomux_v3_cfg_t init_pads[] = {
	/* ECSPI1 */
	IOMUX_PAD_CTRL(EIM_D17__ECSPI1_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D18__ECSPI1_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D16__ECSPI1_SCLK, SPI_PAD_CTRL),
#define GP_ECSPI1_NOR_CS	IMX_GPIO_NR(3, 19)
	IOMUX_PAD_CTRL(EIM_D19__GPIO3_IO19, WEAK_PULLUP),	/* SS1 */

	/* ECSPI3 - GS2971 */
	IOMUX_PAD_CTRL(DISP0_DAT2__ECSPI3_MISO, SPI_PAD_CTRL),	/* pin E7 - SDOUT */
	IOMUX_PAD_CTRL(DISP0_DAT1__ECSPI3_MOSI, SPI_PAD_CTRL),	/* pin E8 - SDIN */
	IOMUX_PAD_CTRL(DISP0_DAT0__ECSPI3_SCLK, SPI_PAD_CTRL),	/* pin F8 - SCLK */
#define GP_ECSPI3_GS2971_CS	IMX_GPIO_NR(4, 24)
	IOMUX_PAD_CTRL(DISP0_DAT3__GPIO4_IO24, WEAK_PULLUP),	/* 0 - pin F7 - CS0 */

	/* ENET pads that don't change for PHY reset */
	IOMUX_PAD_CTRL(ENET_MDIO__ENET_MDIO, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(ENET_MDC__ENET_MDC, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TXC__RGMII_TXC, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD0__RGMII_TD0, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD1__RGMII_TD1, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD2__RGMII_TD2, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD3__RGMII_TD3, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TX_CTL__RGMII_TX_CTL, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(ENET_REF_CLK__ENET_TX_CLK, ENET_PAD_CTRL),
	/* pin 42 PHY nRST */
#define GP_ENET_PHY_RESET	IMX_GPIO_NR(1, 27)
	IOMUX_PAD_CTRL(ENET_RXD0__GPIO1_IO27, OUTPUT_40OHM),
#define GPIRQ_ENET_PHY		IMX_GPIO_NR(1, 28)
	IOMUX_PAD_CTRL(ENET_TX_EN__GPIO1_IO28, WEAK_PULLUP),	/* Micrel RGMII Phy Interrupt */

	/* gpios J91  */
	/* grounds, 1,2,11,12,29,46,47,48,49,50 */
#define GP_BT_GPIO1	IMX_GPIO_NR(2, 15)
	IOMUX_PAD_CTRL(SD4_DAT7__GPIO2_IO15, WEAK_PULLUP),	/* bt_gpio1, pin 3 */
#define GP_BT_GPIO2	IMX_GPIO_NR(2, 14)
	IOMUX_PAD_CTRL(SD4_DAT6__GPIO2_IO14, WEAK_PULLUP),	/* bt_gpio2, pin 4 */
#define GP_BT_GPIO3	IMX_GPIO_NR(2, 13)
	IOMUX_PAD_CTRL(SD4_DAT5__GPIO2_IO13, WEAK_PULLUP),	/* bt_gpio3, pin 5 */
#define GP_BT_GPIO4	IMX_GPIO_NR(2, 12)
	IOMUX_PAD_CTRL(SD4_DAT4__GPIO2_IO12, WEAK_PULLUP),	/* bt_gpio4, pin 6 */
#define GP_BT_GPIO5	IMX_GPIO_NR(2, 11)
	IOMUX_PAD_CTRL(SD4_DAT3__GPIO2_IO11, WEAK_PULLUP),	/* bt_gpio5, pin 7 */
#define GP_BT_GPIO6	IMX_GPIO_NR(2, 10)
	IOMUX_PAD_CTRL(SD4_DAT2__GPIO2_IO10, WEAK_PULLUP),	/* bt_gpio6, pin 8 */
#define GP_BT_GPIO7	IMX_GPIO_NR(2, 9)
	IOMUX_PAD_CTRL(SD4_DAT1__GPIO2_IO09, WEAK_PULLUP),	/* bt_gpio7, pin 9 */
#define GP_BT_GPIO8	IMX_GPIO_NR(2, 8)
	IOMUX_PAD_CTRL(SD4_DAT0__GPIO2_IO08, WEAK_PULLUP),	/* bt_gpio8, pin 10 */
#define GP_BT_GPIO9	IMX_GPIO_NR(7, 9)
	IOMUX_PAD_CTRL(SD4_CMD__GPIO7_IO09, WEAK_PULLUP),	/* bt_gpio9, pin 13 */
#define GP_BT_GPIO10	IMX_GPIO_NR(7, 10)
	IOMUX_PAD_CTRL(SD4_CLK__GPIO7_IO10, WEAK_PULLUP),	/* bt_gpio10, pin 14 */
#define GP_BT_GPIO11	IMX_GPIO_NR(2, 7)
	IOMUX_PAD_CTRL(NANDF_D7__GPIO2_IO07, WEAK_PULLUP),	/* bt_gpio11, pin 15 */
#define GP_BT_GPIO12	IMX_GPIO_NR(2, 6)
	IOMUX_PAD_CTRL(NANDF_D6__GPIO2_IO06, WEAK_PULLUP),	/* bt_gpio12, pin 16 */
#define GP_BT_GPIO13	IMX_GPIO_NR(2, 5)
	IOMUX_PAD_CTRL(NANDF_D5__GPIO2_IO05, WEAK_PULLUP),	/* bt_gpio13, pin 17 */
#define GP_BT_GPIO14	IMX_GPIO_NR(2, 4)
	IOMUX_PAD_CTRL(NANDF_D4__GPIO2_IO04, WEAK_PULLUP),	/* bt_gpio14, pin 18 */
#define GP_BT_GPIO15	IMX_GPIO_NR(2, 3)
	IOMUX_PAD_CTRL(NANDF_D3__GPIO2_IO03, WEAK_PULLUP),	/* bt_gpio15, pin 19 */
#define GP_BT_GPIO16	IMX_GPIO_NR(2, 2)
	IOMUX_PAD_CTRL(NANDF_D2__GPIO2_IO02, WEAK_PULLUP),	/* bt_gpio16, pin 20 */
#define GP_BT_GPIO17	IMX_GPIO_NR(2, 1)
	IOMUX_PAD_CTRL(NANDF_D1__GPIO2_IO01, WEAK_PULLUP),	/* bt_gpio17, pin 21 */
#define GP_BT_GPIO18	IMX_GPIO_NR(2, 0)
	IOMUX_PAD_CTRL(NANDF_D0__GPIO2_IO00, WEAK_PULLUP),	/* bt_gpio18, pin 22 */
#define GP_BT_GPIO19	IMX_GPIO_NR(6, 10)
	IOMUX_PAD_CTRL(NANDF_RB0__GPIO6_IO10, WEAK_PULLUP),	/* bt_gpio19, pin 23 */
#define GP_BT_GPIO20	IMX_GPIO_NR(6, 9)
	IOMUX_PAD_CTRL(NANDF_WP_B__GPIO6_IO09, WEAK_PULLUP),	/* bt_gpio20, pin 24 */
#define GP_BT_GPIO21	IMX_GPIO_NR(6, 7)
	IOMUX_PAD_CTRL(NANDF_CLE__GPIO6_IO07, WEAK_PULLUP),	/* bt_gpio21, pin 25 */
#define GP_BT_GPIO22	IMX_GPIO_NR(6, 8)
	IOMUX_PAD_CTRL(NANDF_ALE__GPIO6_IO08, WEAK_PULLUP),	/* bt_gpio22, pin 26 */
#define GP_BT_GPIO23	IMX_GPIO_NR(6, 16)
	IOMUX_PAD_CTRL(NANDF_CS3__GPIO6_IO16, WEAK_PULLUP),	/* bt_gpio23, pin 27 */
#define GP_BT_GPIO24	IMX_GPIO_NR(6, 15)
	IOMUX_PAD_CTRL(NANDF_CS2__GPIO6_IO15, WEAK_PULLUP),	/* bt_gpio24, pin 28 */
#define GP_BT_GPIO25	IMX_GPIO_NR(6, 14)
	IOMUX_PAD_CTRL(NANDF_CS1__GPIO6_IO14, WEAK_PULLUP),	/* bt_gpio25, pin 30 */
#define GP_BT_GPIO26	IMX_GPIO_NR(6, 11)
	IOMUX_PAD_CTRL(NANDF_CS0__GPIO6_IO11, WEAK_PULLUP),	/* bt_gpio26, pin 31 */
#define GP_BT_GPIO27	IMX_GPIO_NR(5, 30)
	IOMUX_PAD_CTRL(CSI0_DAT12__GPIO5_IO30, WEAK_PULLUP),	/* bt_gpio27, pin 32 */
#define GP_BT_GPIO28	IMX_GPIO_NR(5, 31)
	IOMUX_PAD_CTRL(CSI0_DAT13__GPIO5_IO31, WEAK_PULLUP),	/* bt_gpio28, pin 33 */
#define GP_BT_GPIO29	IMX_GPIO_NR(5, 24)
	IOMUX_PAD_CTRL(CSI0_DAT6__GPIO5_IO24, WEAK_PULLUP),	/* bt_gpio29, pin 34 */
#define GP_BT_GPIO30	IMX_GPIO_NR(5, 25)
	IOMUX_PAD_CTRL(CSI0_DAT7__GPIO5_IO25, WEAK_PULLUP),	/* bt_gpio30, pin 35 */
#define GP_BT_GPIO31	IMX_GPIO_NR(6, 2)
	IOMUX_PAD_CTRL(CSI0_DAT16__GPIO6_IO02, WEAK_PULLUP),	/* bt_gpio31, pin 36 */
#define GP_BT_GPIO32	IMX_GPIO_NR(6, 3)
	IOMUX_PAD_CTRL(CSI0_DAT17__GPIO6_IO03, WEAK_PULLUP),	/* bt_gpio32, pin 37 */
#define GP_BT_GPIO33	IMX_GPIO_NR(6, 4)
	IOMUX_PAD_CTRL(CSI0_DAT18__GPIO6_IO04, WEAK_PULLUP),	/* bt_gpio33, pin 38 */
#define GP_BT_GPIO34	IMX_GPIO_NR(6, 5)
	IOMUX_PAD_CTRL(CSI0_DAT19__GPIO6_IO05, WEAK_PULLUP),	/* bt_gpio34, pin 39 */
#define GP_BT_GPIO35	IMX_GPIO_NR(5, 2)
	IOMUX_PAD_CTRL(EIM_A25__GPIO5_IO02, WEAK_PULLUP),	/* bt_gpio35, pin 40 */
#define GP_BT_GPIO36	IMX_GPIO_NR(3, 29)
	IOMUX_PAD_CTRL(EIM_D29__GPIO3_IO29, WEAK_PULLUP),	/* bt_gpio36, pin 41 */
#define GP_BT_GPIO37	IMX_GPIO_NR(2, 30)
	IOMUX_PAD_CTRL(EIM_EB2__GPIO2_IO30, WEAK_PULLUP),	/* bt_gpio37, pin 42 */
#define GP_BT_GPIO38	IMX_GPIO_NR(2, 31)
	IOMUX_PAD_CTRL(EIM_EB3__GPIO2_IO31, WEAK_PULLUP),	/* bt_gpio38, pin 43 */
#define GP_BT_GPIO39	IMX_GPIO_NR(5, 26)
	IOMUX_PAD_CTRL(CSI0_DAT8__GPIO5_IO26, WEAK_PULLUP),	/* bt_gpio39, pin 44 */
#define GP_BT_GPIO40	IMX_GPIO_NR(5, 27)
	IOMUX_PAD_CTRL(CSI0_DAT9__GPIO5_IO27, WEAK_PULLUP),	/* bt_gpio40, pin 45 */

	/* Power control, high is off */
	/* Pull-up so that reset will leave high */
#define GP_PWR_J1	IMX_GPIO_NR(5, 9)
	IOMUX_PAD_CTRL(DISP0_DAT15__GPIO5_IO09, WEAK_PULLUP),	/* J1 Power enable */
#define GP_PWR_J2	IMX_GPIO_NR(4, 25)
	IOMUX_PAD_CTRL(DISP0_DAT4__GPIO4_IO25, WEAK_PULLUP),	/* J2 */
#define GP_PWR_J3	IMX_GPIO_NR(2, 23)
	IOMUX_PAD_CTRL(EIM_CS0__GPIO2_IO23, WEAK_PULLUP),	/* J3 */
#define GP_PWR_J4	IMX_GPIO_NR(2, 25)
	IOMUX_PAD_CTRL(EIM_OE__GPIO2_IO25, WEAK_PULLUP),	/* J4 */
#define GP_PWR_J6	IMX_GPIO_NR(2, 26)
	IOMUX_PAD_CTRL(EIM_RW__GPIO2_IO26, WEAK_PULLUP),	/* J6 */
#define GP_PWR_J7	IMX_GPIO_NR(2, 27)
	IOMUX_PAD_CTRL(EIM_LBA__GPIO2_IO27, WEAK_PULLUP),	/* J7 */

	/* Dry Contact */
	/* J92 pins */
#define GP_J92_PIN7	IMX_GPIO_NR(3, 31)
	IOMUX_PAD_CTRL(EIM_D31__GPIO3_IO31, WEAK_PULLUP),	/* OUT_1 - Dry contact to J92 pin 7 */
#define GP_J92_PIN9	IMX_GPIO_NR(1, 8)
	IOMUX_PAD_CTRL(GPIO_8__GPIO1_IO08, WEAK_PULLDN),	/* OUT_2 - Dry contact to J92 pin 9 */
#define GP_J92_PIN10	IMX_GPIO_NR(5, 22)
	IOMUX_PAD_CTRL(CSI0_DAT4__GPIO5_IO22, WEAK_PULLUP),	/* GPI_1 - J92 - pin 10 */
#define GP_J92_PIN12	IMX_GPIO_NR(5, 23)
	IOMUX_PAD_CTRL(CSI0_DAT5__GPIO5_IO23, WEAK_PULLUP),	/* GPI_2 - J92 - pin 12 */

	/*
	 * PCIe - tw6869 dedicated,
	 * VIN1-4 used,
	 * AIN1-2 amplified, AIN3-4 not amplified
	 */
#define GP_PCIE_RESET		IMX_GPIO_NR(4, 8)
	IOMUX_PAD_CTRL(KEY_COL1__GPIO4_IO08, OUTPUT_40OHM),

	/* i2c1 rtc rv4162 */
#define GPIRQ_RTC_RV4162	IMX_GPIO_NR(4, 11)
	IOMUX_PAD_CTRL(KEY_ROW2__GPIO4_IO11, WEAK_PULLUP),

	/* I2C3_Adv7391 */
#define GP_ADV7391_RESET	IMX_GPIO_NR(4, 20)
	IOMUX_PAD_CTRL(DI0_PIN4__GPIO4_IO20, WEAK_PULLUP),		/* Adv7391 reset */

	/* SDI - gs2971 on CSI1 */
#if defined(CONFIG_MX6S) || defined(CONFIG_MX6DL)
	/* Dualite/Solo doesn't have IPU2 */
	IOMUX_PAD_CTRL(EIM_A24__IPU1_CSI1_DATA19, CSI_PAD_CTRL),	/* GPIO2[30] */
	IOMUX_PAD_CTRL(EIM_A23__IPU1_CSI1_DATA18, CSI_PAD_CTRL),	/* GPIO6[6] */
	IOMUX_PAD_CTRL(EIM_A22__IPU1_CSI1_DATA17, CSI_PAD_CTRL),	/* GPIO2[16] */
	IOMUX_PAD_CTRL(EIM_A21__IPU1_CSI1_DATA16, CSI_PAD_CTRL),	/* GPIO2[17] */
	IOMUX_PAD_CTRL(EIM_A20__IPU1_CSI1_DATA15, CSI_PAD_CTRL),	/* GPIO2[18] */
	IOMUX_PAD_CTRL(EIM_A19__IPU1_CSI1_DATA14, CSI_PAD_CTRL),	/* GPIO2[19] */
	IOMUX_PAD_CTRL(EIM_A18__IPU1_CSI1_DATA13, CSI_PAD_CTRL),	/* GPIO2[20] */
	IOMUX_PAD_CTRL(EIM_A17__IPU1_CSI1_DATA12, CSI_PAD_CTRL),	/* GPIO2[21] */
	IOMUX_PAD_CTRL(EIM_EB0__IPU1_CSI1_DATA11, CSI_PAD_CTRL),	/* GPIO2[28] */
	IOMUX_PAD_CTRL(EIM_EB1__IPU1_CSI1_DATA10, CSI_PAD_CTRL),	/* GPIO2[29] */
	IOMUX_PAD_CTRL(EIM_DA0__IPU1_CSI1_DATA09, CSI_PAD_CTRL),	/* GPIO3[0] */
	IOMUX_PAD_CTRL(EIM_DA1__IPU1_CSI1_DATA08, CSI_PAD_CTRL),	/* GPIO3[1] */
	IOMUX_PAD_CTRL(EIM_DA2__IPU1_CSI1_DATA07, CSI_PAD_CTRL),	/* GPIO3[2] */
	IOMUX_PAD_CTRL(EIM_DA3__IPU1_CSI1_DATA06, CSI_PAD_CTRL),	/* GPIO3[3] */
	IOMUX_PAD_CTRL(EIM_DA4__IPU1_CSI1_DATA05, CSI_PAD_CTRL),	/* GPIO3[4] */
	IOMUX_PAD_CTRL(EIM_DA5__IPU1_CSI1_DATA04, CSI_PAD_CTRL),	/* GPIO3[5] */
	IOMUX_PAD_CTRL(EIM_DA6__IPU1_CSI1_DATA03, CSI_PAD_CTRL),	/* GPIO3[6] */
	IOMUX_PAD_CTRL(EIM_DA7__IPU1_CSI1_DATA02, CSI_PAD_CTRL),	/* GPIO3[7] */
	IOMUX_PAD_CTRL(EIM_DA8__IPU1_CSI1_DATA01, CSI_PAD_CTRL),	/* GPIO3[8] */
	IOMUX_PAD_CTRL(EIM_DA9__IPU1_CSI1_DATA00, CSI_PAD_CTRL),	/* GPIO3[9] */
	IOMUX_PAD_CTRL(EIM_A16__IPU1_CSI1_PIXCLK, CSI_PAD_CTRL),	/* GPIO2[22] */
#else
	IOMUX_PAD_CTRL(EIM_A24__IPU2_CSI1_DATA19, CSI_PAD_CTRL),	/* GPIO2[30] */
	IOMUX_PAD_CTRL(EIM_A23__IPU2_CSI1_DATA18, CSI_PAD_CTRL),	/* GPIO6[6] */
	IOMUX_PAD_CTRL(EIM_A22__IPU2_CSI1_DATA17, CSI_PAD_CTRL),	/* GPIO2[16] */
	IOMUX_PAD_CTRL(EIM_A21__IPU2_CSI1_DATA16, CSI_PAD_CTRL),	/* GPIO2[17] */
	IOMUX_PAD_CTRL(EIM_A20__IPU2_CSI1_DATA15, CSI_PAD_CTRL),	/* GPIO2[18] */
	IOMUX_PAD_CTRL(EIM_A19__IPU2_CSI1_DATA14, CSI_PAD_CTRL),	/* GPIO2[19] */
	IOMUX_PAD_CTRL(EIM_A18__IPU2_CSI1_DATA13, CSI_PAD_CTRL),	/* GPIO2[20] */
	IOMUX_PAD_CTRL(EIM_A17__IPU2_CSI1_DATA12, CSI_PAD_CTRL),	/* GPIO2[21] */
	IOMUX_PAD_CTRL(EIM_EB0__IPU2_CSI1_DATA11, CSI_PAD_CTRL),	/* GPIO2[28] */
	IOMUX_PAD_CTRL(EIM_EB1__IPU2_CSI1_DATA10, CSI_PAD_CTRL),	/* GPIO2[29] */
	IOMUX_PAD_CTRL(EIM_DA0__IPU2_CSI1_DATA09, CSI_PAD_CTRL),	/* GPIO3[0] */
	IOMUX_PAD_CTRL(EIM_DA1__IPU2_CSI1_DATA08, CSI_PAD_CTRL),	/* GPIO3[1] */
	IOMUX_PAD_CTRL(EIM_DA2__IPU2_CSI1_DATA07, CSI_PAD_CTRL),	/* GPIO3[2] */
	IOMUX_PAD_CTRL(EIM_DA3__IPU2_CSI1_DATA06, CSI_PAD_CTRL),	/* GPIO3[3] */
	IOMUX_PAD_CTRL(EIM_DA4__IPU2_CSI1_DATA05, CSI_PAD_CTRL),	/* GPIO3[4] */
	IOMUX_PAD_CTRL(EIM_DA5__IPU2_CSI1_DATA04, CSI_PAD_CTRL),	/* GPIO3[5] */
	IOMUX_PAD_CTRL(EIM_DA6__IPU2_CSI1_DATA03, CSI_PAD_CTRL),	/* GPIO3[6] */
	IOMUX_PAD_CTRL(EIM_DA7__IPU2_CSI1_DATA02, CSI_PAD_CTRL),	/* GPIO3[7] */
	IOMUX_PAD_CTRL(EIM_DA8__IPU2_CSI1_DATA01, CSI_PAD_CTRL),	/* GPIO3[8] */
	IOMUX_PAD_CTRL(EIM_DA9__IPU2_CSI1_DATA00, CSI_PAD_CTRL),	/* GPIO3[9] */
	IOMUX_PAD_CTRL(EIM_A16__IPU2_CSI1_PIXCLK, CSI_PAD_CTRL),	/* GPIO2[22] - pin A8 */
#endif

	/* Not used, but MUST be in GPIO mode */
	IOMUX_PAD_CTRL(EIM_DA10__GPIO3_IO10, WEAK_PULLUP),	/* IPU2_CSI1_DATA_EN not used (pin B5 stat2) */
	IOMUX_PAD_CTRL(EIM_DA11__GPIO3_IO11, WEAK_PULLUP),	/* HSYNC - pin A5 stat0 */
	IOMUX_PAD_CTRL(EIM_DA12__GPIO3_IO12, WEAK_PULLUP),	/* VSYNC - pin A6 stat1 */

#define GP_GS2971_SMPTE_BYPASS	IMX_GPIO_NR(2, 24)
	IOMUX_PAD_CTRL(EIM_CS1__GPIO2_IO24, WEAK_PULLUP),	/* pin G7 - i/o SMPTE bypass */
#define GP_GS2971_RESET		IMX_GPIO_NR(3, 13)
	IOMUX_PAD_CTRL(EIM_DA13__GPIO3_IO13, OUTPUT_40OHM),	/* 0 - pin C7 - reset */
#define GP_GS2971_DVI_LOCK	IMX_GPIO_NR(3, 14)
	IOMUX_PAD_CTRL(EIM_DA14__GPIO3_IO14, WEAK_PULLUP),	/* pin B6 - stat3 - DVI_LOCK */
#define GP_GS2971_DATA_ERR	IMX_GPIO_NR(3, 15)
	IOMUX_PAD_CTRL(EIM_DA15__GPIO3_IO15, WEAK_PULLUP),	/* pin C6 - stat5 - DATA error */
#define GP_GS2971_LB_CONT	IMX_GPIO_NR(3, 20)
	IOMUX_PAD_CTRL(EIM_D20__GPIO3_IO20, WEAK_PULLUP),	/* pin A3 - LB control - float, analog input */
#define GP_GS2971_Y_1ANC	IMX_GPIO_NR(4, 26)
	IOMUX_PAD_CTRL(DISP0_DAT5__GPIO4_IO26, WEAK_PULLUP),	/* pin C5 - stat4 - 1ANC - Y signal detect */
#define GP_GS2971_RC_BYPASS	IMX_GPIO_NR(4, 27)
	IOMUX_PAD_CTRL(DISP0_DAT6__GPIO4_IO27, OUTPUT_40OHM),	/* 0 - pin G3 - RC bypass - output is buffered(low) */
#define GP_GS2971_IOPROC_EN	IMX_GPIO_NR(4, 28)
	IOMUX_PAD_CTRL(DISP0_DAT7__GPIO4_IO28, OUTPUT_40OHM),	/* 0 - pin H8 - io(A/V) processor enable */
#define GP_GS2971_AUDIO_EN	IMX_GPIO_NR(4, 29)
	IOMUX_PAD_CTRL(DISP0_DAT8__GPIO4_IO29, OUTPUT_40OHM),	/* 0 - pin H3 - Audio Enable */
#define GP_GS2971_TIM_861	IMX_GPIO_NR(4, 30)
	IOMUX_PAD_CTRL(DISP0_DAT9__GPIO4_IO30, OUTPUT_40OHM),	/* 0 - pin H5 - TIM861 timing format, 0-use HSYNC/VSYNC */
#define GP_GS2971_SW_EN		IMX_GPIO_NR(4, 31)
	IOMUX_PAD_CTRL(DISP0_DAT10__GPIO4_IO31, OUTPUT_40OHM),	/* 0 - pin D7 - SW_EN - line lock enable */
#define GP_GS2971_STANDBY	IMX_GPIO_NR(5, 0)
	IOMUX_PAD_CTRL(EIM_WAIT__GPIO5_IO00, OUTPUT_40OHM),	/* 1 - pin K2 - Standby */
#define GP_GS2971_DVB_ASI	IMX_GPIO_NR(5, 5)
	IOMUX_PAD_CTRL(DISP0_DAT11__GPIO5_IO05, WEAK_PULLUP),	/* pin G8 i/o DVB_ASI */

	IOMUX_PAD_CTRL(KEY_ROW1__AUD5_RXD, AUD_PAD_CTRL),	/* pin J3 - AOUT1/2 */
	IOMUX_PAD_CTRL(DISP0_DAT14__AUD5_RXC, AUD_PAD_CTRL),	/* pin J4 - ACLK*/
	IOMUX_PAD_CTRL(DISP0_DAT13__AUD5_RXFS, AUD_PAD_CTRL),	/* pin H4 - WCLK*/

	/* UART1  */
	IOMUX_PAD_CTRL(CSI0_DAT10__UART1_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT11__UART1_RX_DATA, UART_PAD_CTRL),

	/* UART2 for debug */
	IOMUX_PAD_CTRL(EIM_D26__UART2_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D27__UART2_RX_DATA, UART_PAD_CTRL),

	/* UART3 */
	IOMUX_PAD_CTRL(EIM_D24__UART3_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D25__UART3_RX_DATA, UART_PAD_CTRL),

	/* UART4 */
	IOMUX_PAD_CTRL(KEY_COL0__UART4_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(KEY_ROW0__UART4_RX_DATA, UART_PAD_CTRL),

	/* UART5 */
	IOMUX_PAD_CTRL(CSI0_DAT14__UART5_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT15__UART5_RX_DATA, UART_PAD_CTRL),

	/* UART6/7 on sc16is752 on i2c2 */
#define GP_SC16IS752_IRQ		IMX_GPIO_NR(4, 10)
	IOMUX_PAD_CTRL(KEY_COL2__GPIO4_IO10, WEAK_PULLUP),	/* irq */

	/* USBH1 */
	IOMUX_PAD_CTRL(EIM_D30__USB_H1_OC, WEAK_PULLUP),
#define GP_USB_HUB_RESET	IMX_GPIO_NR(7, 12)
	IOMUX_PAD_CTRL(GPIO_17__GPIO7_IO12, OUTPUT_40OHM),	/* USB Hub Reset for USB2512 4 port hub */
	/*
	 * port1 - 10/100 ethernet using AX88772A on J90
	 * port2 - usb connector on J26
	 * port3 - usb connector on J25
	 * port4 - usb connector on J96
	 */
#define GP_AX88772A_RESET	IMX_GPIO_NR(5, 20)
	IOMUX_PAD_CTRL(CSI0_DATA_EN__GPIO5_IO20, OUTPUT_40OHM),

	/* USBOTG - J80 */
	IOMUX_PAD_CTRL(GPIO_1__USB_OTG_ID, WEAK_PULLUP),
	IOMUX_PAD_CTRL(KEY_COL4__USB_OTG_OC, WEAK_PULLUP),
#define GP_USB_OTG_PWR		IMX_GPIO_NR(3, 22)
	IOMUX_PAD_CTRL(EIM_D22__GPIO3_IO22, OUTPUT_40OHM),

	/* USDHC1: Full size SD card holder - J88 */
	IOMUX_PAD_CTRL(SD1_CLK__SD1_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_CMD__SD1_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT0__SD1_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT1__SD1_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT2__SD1_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT3__SD1_DATA3, USDHC_PAD_CTRL),
#define GP_USDHC1_CD		IMX_GPIO_NR(1, 4)
	IOMUX_PAD_CTRL(GPIO_4__GPIO1_IO04, WEAK_PULLUP),
#define GP_USDHC1_WP		IMX_GPIO_NR(1, 2)
	IOMUX_PAD_CTRL(GPIO_2__GPIO1_IO02, WEAK_PULLUP),
	/* Needs to invert and use key_col1 */
#define GP_USDHC1_POWER_SEL	IMX_GPIO_NR(7, 13)		/* low 1.8V, high 3.3V */
	IOMUX_PAD_CTRL(GPIO_18__GPIO7_IO13, OUTPUT_40OHM),

	/* USDHC2:  micro sd - J87 */
	IOMUX_PAD_CTRL(SD2_CLK__SD2_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_CMD__SD2_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT0__SD2_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT1__SD2_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT2__SD2_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT3__SD2_DATA3, USDHC_PAD_CTRL),
#define GP_USDHC2_CD		IMX_GPIO_NR(3, 23)
	IOMUX_PAD_CTRL(EIM_D23__GPIO3_IO23, WEAK_PULLUP),

	/* USDHC3 - eMMC */
	IOMUX_PAD_CTRL(SD3_CLK__SD3_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_CMD__SD3_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT0__SD3_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT1__SD3_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT2__SD3_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT3__SD3_DATA3, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT4__SD3_DATA4, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT5__SD3_DATA5, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT6__SD3_DATA6, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT7__SD3_DATA7, USDHC_PAD_CTRL),
#define GP_EMMC_RESET	IMX_GPIO_NR(7, 8)
	IOMUX_PAD_CTRL(SD3_RST__GPIO7_IO08, OUTPUT_40OHM),	/* eMMC reset */
};

static const iomux_v3_cfg_t enet_pads1[] = {
	/* pin 35 - 1 (PHY_AD2) on reset */
#define GP_PHY_AD2		IMX_GPIO_NR(6, 30)
	IOMUX_PAD_CTRL(RGMII_RXC__GPIO6_IO30, OUTPUT_40OHM),
	/* pin 32 - 1 - (MODE0) all */
#define GP_PHY_MODE0		IMX_GPIO_NR(6, 25)
	IOMUX_PAD_CTRL(RGMII_RD0__GPIO6_IO25, OUTPUT_40OHM),
	/* pin 31 - 1 - (MODE1) all */
#define GP_PHY_MODE1		IMX_GPIO_NR(6, 27)
	IOMUX_PAD_CTRL(RGMII_RD1__GPIO6_IO27, OUTPUT_40OHM),
	/* pin 28 - 1 - (MODE2) all */
#define GP_PHY_MODE2		IMX_GPIO_NR(6, 28)
	IOMUX_PAD_CTRL(RGMII_RD2__GPIO6_IO28, OUTPUT_40OHM),
	/* pin 27 - 1 - (MODE3) all */
#define GP_PHY_MODE3		IMX_GPIO_NR(6, 29)
	IOMUX_PAD_CTRL(RGMII_RD3__GPIO6_IO29, OUTPUT_40OHM),
	/* pin 33 - 1 - (CLK125_EN) 125Mhz clockout enabled */
#define GP_PHY_CLK125		IMX_GPIO_NR(6, 24)
	IOMUX_PAD_CTRL(RGMII_RX_CTL__GPIO6_IO24, OUTPUT_40OHM),
};

static const iomux_v3_cfg_t enet_pads2[] = {
	IOMUX_PAD_CTRL(RGMII_RXC__RGMII_RXC, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD0__RGMII_RD0, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD1__RGMII_RD1, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD2__RGMII_RD2, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD3__RGMII_RD3, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RX_CTL__RGMII_RX_CTL, ENET_PAD_CTRL),
};

/*
 *
 */
static struct i2c_pads_info i2c_pads[] = {
	/* I2C1, SGTL5000 */
	I2C_PADS_INFO_ENTRY(I2C1, EIM_D21, 3, 21, EIM_D28, 3, 28, I2C_PAD_CTRL),
	/* I2C2 Camera, MIPI */
	I2C_PADS_INFO_ENTRY(I2C2, KEY_COL3, 4, 12, KEY_ROW3, 4, 13, I2C_PAD_CTRL),
	/* I2C3, J15 - RGB connector */
	I2C_PADS_INFO_ENTRY(I2C3, GPIO_5, 1, 05, GPIO_16, 7, 11, I2C_PAD_CTRL),
};

int dram_init(void)
{
	gd->ram_size = ((ulong)CONFIG_DDR_MB * 1024 * 1024);
//	printf("%s:%p *%p=0x%lx\n", __func__, gd, &gd->ram_size, gd->ram_size);
	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	/* Reset USB hub */
	if (port) {
		gpio_set_value(GP_USB_HUB_RESET, 0);
		gpio_set_value(GP_AX88772A_RESET, 0);
		mdelay(2);
		gpio_set_value(GP_USB_HUB_RESET, 1);
		gpio_set_value(GP_AX88772A_RESET, 1);
	}
	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port)
		return 0;
	gpio_set_value(GP_USB_OTG_PWR, on);
	return 0;
}

#endif

#ifdef CONFIG_FSL_ESDHC
static struct fsl_esdhc_cfg usdhc_cfg[] = {
	{.esdhc_base = USDHC1_BASE_ADDR, .max_bus_width = 4},
	{.esdhc_base = USDHC2_BASE_ADDR, .max_bus_width = 4},
	{.esdhc_base = USDHC3_BASE_ADDR, .max_bus_width = 8},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int gp_cd = (cfg->esdhc_base == USDHC1_BASE_ADDR) ? GP_USDHC1_CD :
			GP_USDHC2_CD;

	if (cfg->esdhc_base == USDHC3_BASE_ADDR)
		return 1;	/* eMMC always present */
	return !gpio_get_value(gp_cd);
}

int board_mmc_init(bd_t *bis)
{
	int ret;
	u32 index = 0;

	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);

	for (index = 0; index < CONFIG_SYS_FSL_USDHC_NUM; ++index) {
		switch (index) {
		case 0:
		case 1:
			break;
		case 2:
			gpio_set_value(GP_EMMC_RESET, 1); /* release reset */
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) then supported by the board (%d)\n",
				index + 1, CONFIG_SYS_FSL_USDHC_NUM);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
		if (ret)
			return ret;
	}
	return 0;
}
#endif

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? GP_ECSPI1_NOR_CS : -1;
}

int board_phy_config(struct phy_device *phydev)
{
	/* min rx data delay */
	ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW, 0x0);
	/* min tx data delay */
	ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW, 0x0);
	/* max rx/tx clock delay, min rx/tx control */
	ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_CLOCK_SKEW, 0xf0f0);
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

static void setup_iomux_enet(void)
{
	gpio_direction_output(GP_ENET_PHY_RESET, 0); /* PHY rst */
	gpio_direction_output(GP_PHY_AD2, 1);
	gpio_direction_output(GP_PHY_MODE0, 1);
	gpio_direction_output(GP_PHY_MODE1, 1);
	gpio_direction_output(GP_PHY_MODE2, 1);
	gpio_direction_output(GP_PHY_MODE3, 1);
	gpio_direction_output(GP_PHY_CLK125, 1);
	SETUP_IOMUX_PADS(enet_pads1);

	/* Need delay 10ms according to KSZ9021 spec */
	udelay(1000 * 10);
	gpio_set_value(GP_ENET_PHY_RESET, 1); /* PHY reset */

	SETUP_IOMUX_PADS(enet_pads2);
	udelay(100);	/* Wait 100 us before using mii interface */
}

int board_eth_init(bd_t *bis)
{
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	setup_iomux_enet();

#ifdef CONFIG_FEC_MXC
	bus = fec_get_miibus(base, -1);
	if (!bus)
		return 0;
	/* scan phy 4,5,6,7 */
	phydev = phy_find_by_mask(bus, (0xf << 4), PHY_INTERFACE_MODE_RGMII);
	if (!phydev) {
		free(bus);
		return 0;
	}
	printf("using phy at %d\n", phydev->addr);
	ret  = fec_probe(bis, -1, base, bus, phydev);
	if (ret) {
		printf("FEC MXC: %s:failed\n", __func__);
		free(phydev);
		free(bus);
	}
#endif

#ifdef CONFIG_CI_UDC
	/* For otg ethernet*/
	usb_eth_initialize(bis);
#endif
	return 0;
}


int splash_screen_prepare(void)
{
	char *env_loadsplash;

	if (!getenv("splashimage") || !getenv("splashsize")) {
		return -1;
	}

	env_loadsplash = getenv("loadsplash");
	if (env_loadsplash == NULL) {
		printf("Environment variable loadsplash not found!\n");
		return -1;
	}

	if (run_command_list(env_loadsplash, -1, 0)) {
		printf("failed to run loadsplash %s\n\n", env_loadsplash);
		return -1;
	}

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)

static const struct display_info_t displays[] = {
	/* hdmi */
	IMX_VD50_1280_720M_60(HDMI, 1, 1),
	IMX_VD50_1920_1080M_60(HDMI, 0, 1),
	IMX_VD50_1024_768M_60(HDMI, 0, 1),
};

int board_cfb_skip(void)
{
	return NULL != getenv("novideo");
}
#endif


static const unsigned short gpios_out_low[] = {
	GP_EMMC_RESET,		/* hold in reset */
	GP_USB_OTG_PWR,		/* disable USB otg power */
	GP_USB_HUB_RESET,	/* disable hub */
	GP_AX88772A_RESET,
	GP_ENET_PHY_RESET,
	GP_PCIE_RESET,
	GP_GS2971_RESET,
	GP_GS2971_RC_BYPASS,
	GP_GS2971_IOPROC_EN,
	GP_GS2971_AUDIO_EN,
	GP_GS2971_TIM_861,
	GP_GS2971_SW_EN,
	GP_GS2971_DVB_ASI,
	GP_ADV7391_RESET,
	GP_J92_PIN9,
};

static const unsigned short gpios_out_high[] = {
	GP_PWR_J1,
	GP_PWR_J2,
	GP_PWR_J3,
	GP_PWR_J4,
	GP_PWR_J6,
	GP_PWR_J7,
	GP_ECSPI1_NOR_CS,	/* SS1 of spi nor */
	GP_ECSPI3_GS2971_CS,
	GP_USDHC1_POWER_SEL,	/* high=3.3v */
	GP_GS2971_STANDBY,
	GP_J92_PIN7,
};

static const unsigned short gpios_in[] = {
	GP_USDHC1_CD,
	GP_USDHC2_CD,
	GP_GS2971_SMPTE_BYPASS,
	GP_GS2971_DVI_LOCK,
	GP_GS2971_DATA_ERR,
	GP_GS2971_LB_CONT,
	GP_GS2971_Y_1ANC,
	GP_J92_PIN10,
	GP_J92_PIN12,
	GP_BT_GPIO1,
	GP_BT_GPIO2,
	GP_BT_GPIO3,
	GP_BT_GPIO4,
	GP_BT_GPIO5,
	GP_BT_GPIO6,
	GP_BT_GPIO7,
	GP_BT_GPIO8,
	GP_BT_GPIO9,
	GP_BT_GPIO10,
	GP_BT_GPIO11,
	GP_BT_GPIO12,
	GP_BT_GPIO13,
	GP_BT_GPIO14,
	GP_BT_GPIO15,
	GP_BT_GPIO16,
	GP_BT_GPIO17,
	GP_BT_GPIO18,
	GP_BT_GPIO19,
	GP_BT_GPIO20,
	GP_BT_GPIO21,
	GP_BT_GPIO22,
	GP_BT_GPIO23,
	GP_BT_GPIO24,
	GP_BT_GPIO25,
	GP_BT_GPIO26,
	GP_BT_GPIO27,
	GP_BT_GPIO28,
	GP_BT_GPIO29,
	GP_BT_GPIO30,
	GP_BT_GPIO31,
	GP_BT_GPIO32,
	GP_BT_GPIO33,
	GP_BT_GPIO34,
	GP_BT_GPIO35,
	GP_BT_GPIO36,
	GP_BT_GPIO37,
	GP_BT_GPIO38,
	GP_BT_GPIO39,
	GP_BT_GPIO40,
};

static void set_gpios_in(const unsigned short *p, int cnt)
{
	int i;

	for (i = 0; i < cnt; i++)
		gpio_direction_input(*p++);
}

static void set_gpios(const unsigned short *p, int cnt, int val)
{
	int i;

	for (i = 0; i < cnt; i++)
		gpio_direction_output(*p++, val);
}

int board_early_init_f(void)
{
	set_gpios_in(gpios_in, ARRAY_SIZE(gpios_in));
	set_gpios(gpios_out_high, ARRAY_SIZE(gpios_out_high), 1);
	set_gpios(gpios_out_low, ARRAY_SIZE(gpios_out_low), 0);
	SETUP_IOMUX_PADS(init_pads);
	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_init(void)
{
	int i;
	struct i2c_pads_info *p = i2c_pads + i2c_get_info_entry_offset();
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	clrsetbits_le32(&iomuxc_regs->gpr[1],
			IOMUXC_GPR1_OTG_ID_MASK,
			IOMUXC_GPR1_OTG_ID_GPIO1);

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
	for (i = 0; i < 3; i++) {
	        setup_i2c(i, CONFIG_SYS_I2C_SPEED, 0x7f, p);
		p += I2C_PADS_INFO_ENTRY_SPACING;
	}

#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif

#ifdef CONFIG_CMD_FBPANEL
	fbp_setup_display(displays, ARRAY_SIZE(displays));
#endif
	return 0;
}


int checkboard(void)
{
	puts("Board: BT\n");
	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{"mmc1",	MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
	{NULL,		0},
};
#endif

int misc_init_r(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	return 0;
}

int board_late_init(void)
{
	int cpurev = get_cpu_rev();

	setenv("cpu", get_imx_type((cpurev & 0xFF000) >> 12));
	if (!getenv("board"))
		setenv("board", "bt");
	if (!getenv("uboot_defconfig"))
		setenv("uboot_defconfig", CONFIG_DEFCONFIG);
	return 0;
}
