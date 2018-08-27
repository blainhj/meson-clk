// SPDX-License-Identifier: GPL-2.0+
/*
 * Amlogic Meson-G12A Clock Controller Driver
 *
 * Copyright (c) 2016 Baylibre SAS.
 * Author: Michael Turquette <mturquette@baylibre.com>
 *
 * Copyright (c) 2018 Amlogic, inc.
 * Author: Jian Hu <jian.hu@amlogic.com>
 */
#include <linux/clk-provider.h>
#include <linux/platform_device.h>
#include <linux/reset-controller.h>
#include <linux/mfd/syscon.h>
#include <linux/init.h>
#include "clkc.h"
#include "meson-aoclk.h"
#include "g12a-aoclk.h"

#define G12A_AO_GATE(_name, _bit)					\
static struct clk_regmap g12a_aoclk_##_name = {					\
	.data = &(struct clk_regmap_gate_data) {			\
		.offset = (AO_CLK_GATE0),			\
		.bit_idx = (_bit),					\
	},								\
	.hw.init = &(struct clk_init_data) {				\
		.name = "g12a_ao_" #_name,					\
		.ops = &clk_regmap_gate_ops,				\
		.parent_names = (const char *[]){ "clk81" },		\
		.num_parents = 1,					\
	},								\
}

G12A_AO_GATE(ahb_bus,		0);
G12A_AO_GATE(remote,		1);
G12A_AO_GATE(i2c_master,	2);
G12A_AO_GATE(i2c_slave,	3);
G12A_AO_GATE(uart1,		4);
G12A_AO_GATE(prod_i2c,		5);
G12A_AO_GATE(uart2,		6);
G12A_AO_GATE(ir_blaster,	7);
G12A_AO_GATE(saradc,		8);

static struct clk_regmap g12a_aoclk_clk81 = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = AO_RTI_PWR_CNTL_REG0,
		.mask = 0x1,
		.shift = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "g12a_ao_clk81",
		.ops = &clk_regmap_mux_ro_ops,
		.parent_names = (const char *[]){ "clk81", "ao_alt_xtal"},
		.num_parents = 2,
	},
};

static struct clk_regmap g12a_aoclk_saradc_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = AO_SAR_CLK,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data){
		.name = "g12a_ao_saradc_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_names = (const char *[]){ "xtal", "g12a_ao_clk81" },
		.num_parents = 2,
	},
};

static struct clk_regmap g12a_aoclk_saradc_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = AO_SAR_CLK,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "g12a_ao_saradc_div",
		.ops = &clk_regmap_divider_ops,
		.parent_names = (const char *[]){ "g12a_ao_saradc_mux" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap g12a_aoclk_saradc_gate = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = AO_SAR_CLK,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "g12a_ao_saradc_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_names = (const char *[]){ "g12a_ao_saradc_div" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static unsigned int g12a_aoclk_reset[] = {
	[RESET_AO_REMOTE] =	16,
	[RESET_AO_UART1] =	17,
	[RESET_AO_I2C_MASTER] = 18,
	[RESET_AO_I2C_SLAVE] =	19,
	[RESET_AO_SARADC] =	20,
	[RESET_AO_UART2] =	22,
	[RESET_AO_IR_BLASTER] = 23,
};

static struct clk_regmap *g12a_aoclk_regmap[] = {
	[CLKID_AO_AHB_BUS]	= &g12a_aoclk_ahb_bus,
	[CLKID_AO_REMOTE]	= &g12a_aoclk_remote,
	[CLKID_AO_I2C_MASTER]	= &g12a_aoclk_i2c_master,
	[CLKID_AO_I2C_SLAVE]	= &g12a_aoclk_i2c_slave,
	[CLKID_AO_UART1]	= &g12a_aoclk_uart1,
	[CLKID_AO_PROD_I2C]	= &g12a_aoclk_prod_i2c,
	[CLKID_AO_UART2]	= &g12a_aoclk_uart2,
	[CLKID_AO_IR_BLASTER]	= &g12a_aoclk_ir_blaster,
	[CLKID_AO_SAR_ADC]	= &g12a_aoclk_saradc,
	[CLKID_AO_CLK81]	= &g12a_aoclk_clk81,
	[CLKID_AO_SAR_ADC_SEL]	= &g12a_aoclk_saradc_mux,
	[CLKID_AO_SAR_ADC_DIV]	= &g12a_aoclk_saradc_div,
	[CLKID_AO_SAR_ADC_CLK]	= &g12a_aoclk_saradc_gate,
};

static struct clk_hw_onecell_data g12a_aoclk_onecell_data = {
	.hws = {
		[CLKID_AO_AHB_BUS]	= &g12a_aoclk_ahb_bus.hw,
		[CLKID_AO_REMOTE]	= &g12a_aoclk_remote.hw,
		[CLKID_AO_I2C_MASTER]	= &g12a_aoclk_i2c_master.hw,
		[CLKID_AO_I2C_SLAVE]	= &g12a_aoclk_i2c_slave.hw,
		[CLKID_AO_UART1]	= &g12a_aoclk_uart1.hw,
		[CLKID_AO_PROD_I2C]	= &g12a_aoclk_prod_i2c.hw,
		[CLKID_AO_UART2]	= &g12a_aoclk_uart2.hw,
		[CLKID_AO_IR_BLASTER]	= &g12a_aoclk_ir_blaster.hw,
		[CLKID_AO_SAR_ADC]	= &g12a_aoclk_saradc.hw,
		[CLKID_AO_CLK81]	= &g12a_aoclk_clk81.hw,
		[CLKID_AO_SAR_ADC_SEL]	= &g12a_aoclk_saradc_mux.hw,
		[CLKID_AO_SAR_ADC_DIV]	= &g12a_aoclk_saradc_div.hw,
		[CLKID_AO_SAR_ADC_CLK]	= &g12a_aoclk_saradc_gate.hw,
	},
	.num = NR_CLKS,
};

static struct meson_aoclk_data g12a_aoclkc_data = {
	.reset_reg	= AO_RTI_GEN_CNTL_REG0,
	.num_reset	= ARRAY_SIZE(g12a_aoclk_reset),
	.reset		= g12a_aoclk_reset,
	.num_clks	= ARRAY_SIZE(g12a_aoclk_regmap),
	.clks		= g12a_aoclk_regmap,
	.hw_data	= &g12a_aoclk_onecell_data,
};

static const struct of_device_id g12a_aoclkc_match_table[] = {
	{
		.compatible	= "amlogic,g12a-aoclkc",
		.data		= &g12a_aoclkc_data,
	},
	{ }
};

static struct platform_driver g12a_aoclkc_driver = {
	.probe		= meson_aoclkc_probe,
	.driver		= {
		.name	= "g12a-aoclkc",
		.of_match_table = g12a_aoclkc_match_table,
	},
};

builtin_platform_driver(g12a_aoclkc_driver);
