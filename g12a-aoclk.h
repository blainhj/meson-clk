/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2017 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 * Copyright (c) 2018 Amlogic, inc.
 * Author: Jian Hu <Jian.Hu@amlogic.com>
 */

#ifndef __G12A_AOCLKC_H
#define __G12A_AOCLKC_H

//#include "meson-aoclk.h"

#define NR_CLKS				14

/* AO Configuration Clock registers offsets
 * Register offsets from the data sheet must be multiplied by 4.
 */
#define AO_RTI_PWR_CNTL_REG0		0x10
#define AO_RTI_GEN_CNTL_REG0		0x40
#define AO_OSCIN_CNTL			0x58
#define AO_CRT_CLK_CNTL1		0x68
#define AO_SAR_CLK			0x90
#define AO_RTC_ALT_CLK_CNTL0		0x94
#define AO_RTC_ALT_CLK_CNTL1		0x98

/*
 *AO CLK81 gate clocks
 */
#define AO_CLK_GATE0			0x4c

#include <dt-bindings/clock/g12a-aoclkc.h>
#include <dt-bindings/reset/g12a-aoclkc.h>

#endif /* __G12A_AOCLKC_H */
