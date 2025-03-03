// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 STMicroelectronics - All Rights Reserved
 * Author(s): Yannick Fertre <yannick.fertre@st.com> for STMicroelectronics.
 *            Philippe Cornu <philippe.cornu@st.com> for STMicroelectronics.
 *
 * This rm68200 panel driver is inspired from the Linux Kernel driver
 * drivers/gpu/drm/panel/panel-raydium-rm68200.c.
 */
#include <backlight.h>
#include <dm.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <power/regulator.h>

struct rm68200_panel_priv {
	struct udevice *reg;
	struct udevice *backlight;
	struct gpio_desc reset;
};

 static const struct display_timing default_timing = {
	.pixelclock.typ		= 25000000,  // 25 MHz (25000 kHz)
	.hactive.typ		= 400,       // Yatay aktif çözünürlük
	.hfront_porch.typ	= 46,        // Yatay front porch (HFP)
	.hback_porch.typ	= 44,        // Yatay back porch (HBP)
	.hsync_len.typ		= 2,         // Yatay sync width (HS)
	.vactive.typ		= 1200,      // Dikey aktif çözünürlük
	.vfront_porch.typ	= 16,        // Dikey front porch (VFP)
	.vback_porch.typ	= 14,        // Dikey back porch (VBP)
	.vsync_len.typ		= 2,         // Dikey sync width (VS)
};

struct panel_init_cmd {
	u8 dtype;
	u8 wait;
	u8 dlen;
	const char *data;
};

/*
 * Display initilization sequence provided by manufacturer.
 */
 static const struct panel_init_cmd afj101_ba2131_init_cmds[] = {
        { .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x30}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x52}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x01}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xE3,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x24,0x10}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x25,0x0A}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x28,0xB4}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x29,0x44}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x2a,0xff}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x37,0x74}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x38,0x7f}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x39,0x3F}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x49,0x3C}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x59,0xfe}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x5C,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x80,0x20}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x91,0x77}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x92,0x77}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x99,0x54}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x9B,0x56}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xA0,0x55}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xA1,0x50}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xA4,0x9C}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xA7,0x02}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xA8,0x01}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xA9,0x21}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xAA,0xA8}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xAB,0x28}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xAC,0xE0}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xAD,0xE2}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xAE,0xE2}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xAF,0x02}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB0,0xE2}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB1,0x26}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB2,0x28}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB3,0x28}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB4,0x22}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB5,0xE2}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB6,0x26}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB7,0xE2}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB8,0x26}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xF0,0x00}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xF6,0xC0}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xc3,0x0f}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x2c,0x03}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x30}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x52}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x02}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB1,0x05}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xD1,0x05}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB4,0x2C}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xD4,0x2A}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB2,0x01}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xD2,0x01}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB3,0x29}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xD3,0x27}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB6,0x07}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xD6,0x05}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB7,0x2C}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xD7,0x2A}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xC1,0x05}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xE1,0x05}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB8,0x0A}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xD8,0x0A}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB9,0x01}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xD9,0x01}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xBD,0x14}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xDD,0x14}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xBC,0x12}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xDC,0x12}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xBB,0x10}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xDB,0x10}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xBA,0x10}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xDA,0x10}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xBE,0x17}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xDE,0x19}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xBF,0x0E}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xDF,0x10}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xC0,0x17}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xE0,0x19}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB5,0x37}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xD5,0x32}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xB0,0x02}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xD0,0x05}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x30}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x52}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x03}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x00,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x01,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x02,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x03,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x08,0x89}},    
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x09,0x8a}},   
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x0A,0x87}},   
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x0B,0x88}},   
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x20,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x21,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x22,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x23,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x2A,0x05}},   
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x2B,0x05}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x30,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x31,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x32,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x33,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x34,0x01}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x35,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x36,0x00}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x37,0x03}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x40,0x85}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x41,0x86}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x42,0x83}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x43,0x84}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x44,0x44}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x45,0xFF}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x46,0xFE}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x48,0x01}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x49,0x00}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x50,0x81}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x51,0x82}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x52,0x01}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x53,0x80}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x55,0x03}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x56,0x02}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x58,0x05}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x59,0x04}},  
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x60,0x8e}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x61,0x8e}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x64,0x77}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x65,0xf7}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x66,0xf7}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x80,0x0f}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x81,0x0E}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x82,0x0f}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x88,0x0f}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x83,0x08}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x89,0x0e}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x84,0x06}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x85,0x07}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x86,0x04}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x87,0x05}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x8A,0x00}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x8B,0x01}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x96,0x0f}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x97,0x0e}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x98,0x0f}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x9E,0x0f}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x99,0x08}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x9F,0x0e}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x9A,0x06}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x9B,0x07}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x9C,0x04}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x9D,0x05}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xA0,0x00}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xA1,0x01}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x30}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x52}},
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0xFF,0x00}}, 
	{ .dtype = 0x15, .wait = 0x00, .dlen = 0x02, .data = (char[]){ 0x36,0x00}},
	{ .dtype = 0x15, .wait = 220,  .dlen = 0x02, .data = (char[]){ 0x11,0x00}},
	{ .dtype = 0x15, .wait = 120,  .dlen = 0x02, .data = (char[]){ 0x29,0x00}},	
};
 
static int rm68200_init_sequence(struct udevice *dev)
{
	unsigned int i;
	int ret;

 	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
        struct rm68200_panel_priv *priv = dev_get_priv(dev);
	for (i = 0; i < ARRAY_SIZE(afj101_ba2131_init_cmds); i++) {
		const struct panel_init_cmd *cmd = &afj101_ba2131_init_cmds[i];

		switch (cmd->dtype) {
			case MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM:
			case MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM:
			case MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM:
			case MIPI_DSI_GENERIC_LONG_WRITE:  
			case MIPI_DSI_DCS_SHORT_WRITE:
			case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
			case MIPI_DSI_DCS_LONG_WRITE:
				ret = mipi_dsi_dcs_write_buffer(device, cmd->data, cmd->dlen);								
				break;
			default:
			        dev_err(dev, "unsupported mipi dsi command, exiting\n");
				goto powerdown;
		}

		if (ret < 0){
		      dev_err(dev, "Critical error, mipi_dsi_dcs_write_buffer func() returns err: %d\n", ret);
		      goto powerdown;
		}
			
		if (cmd->wait)
			mdelay(cmd->wait);
	}
	return 0;
powerdown:
      dev_info(dev, "shutdown the panel due to initialization error \n");
      dm_gpio_set_value(&priv->reset, false);
      mdelay(50);
      return 1;
}

static int rm68200_panel_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	struct rm68200_panel_priv *priv = dev_get_priv(dev);
	int ret;
        dev_info(dev, "Panel set backlight func(), initializing panel \n");
        if(device == NULL){
             dev_err(dev, "Critical error, MIPI DSI Host not initialized, exiting, panel will not work.\n");
             goto powerdown;
        }
        dev_info(dev, "attaching mipi dsi panel\n");
	ret = mipi_dsi_attach(device);
	if (ret < 0){
	     dev_err(dev, "Critical error, mipi dsi attach func() returns err: %d\n", ret);
             goto powerdown;
	}
        dev_info(dev, "perform panel initialization sequence\n");
	if(rm68200_init_sequence(dev)){
	     dev_err(dev, "panel initialization failed");
             goto powerdown;
	}
        dev_info(dev, "panel initialization sequence done, panel probed!\n");
	return 0; 
powerdown:
      dev_info(dev, "shutdown the panel due to initialization error \n");
      dm_gpio_set_value(&priv->reset, false);
      mdelay(50);
      return 1;
}
 
static int rm68200_panel_get_display_timing(struct udevice *dev,
					    struct display_timing *timings)
{
	memcpy(timings, &default_timing, sizeof(*timings));
	return 0;
}

static int rm68200_panel_of_to_plat(struct udevice *dev)
{
	struct rm68200_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_by_name(dev, "reset-gpios", 0, &priv->reset,  GPIOD_IS_OUT);
				 
	if (ret) {
		dev_err(dev, "Warning, couldnt get reset gpio, check dts file if required, err %d\n", ret);
		if (ret != -ENOENT)
			return ret;
	}
        dev_info(dev, "reset pin set done\n");
	return 0;
}

static int rm68200_panel_probe(struct udevice *dev)
{
	struct rm68200_panel_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	dev_info(dev, "Initializing MIPI DSI Panel Driver\n");
	/* reset panel */
	dm_gpio_set_value(&priv->reset, true);
	mdelay(50);
	dm_gpio_set_value(&priv->reset, false);
	mdelay(15);
	dm_gpio_set_value(&priv->reset, true);
        dev_info(dev, "Reset sequence done \n");
	/* fill characteristics of DSI data link */
	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST;
        dev_info(dev, "dsi set 4 lane, video-burst, RGB 24-bit mode\n");
	return 0;
}

static const struct panel_ops rm68200_panel_ops = {
	.enable_backlight = rm68200_panel_enable_backlight,
	.get_display_timing = rm68200_panel_get_display_timing,
};

static const struct udevice_id rm68200_panel_ids[] = {
	{ .compatible = "raydium,rm68200" },
	{ }
};

U_BOOT_DRIVER(rm68200_panel) = {
	.name			  = "rm68200_panel",
	.id			  = UCLASS_PANEL,
	.of_match		  = rm68200_panel_ids,
	.ops			  = &rm68200_panel_ops,
	.of_to_plat	  = rm68200_panel_of_to_plat,
	.probe			  = rm68200_panel_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct rm68200_panel_priv),
};
