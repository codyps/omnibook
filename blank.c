/*
 * blank.c -- blanking lcd console
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Written by Soós Péter <sp@osb.hu>, 2002-2004
 * Modified by Mathieu Bérard <mathieu.berard@crans.org>, 2006
 */

#include "omnibook.h"

#include <asm/io.h>
#include "ec.h"

static struct omnibook_feature blank_driver;

static int omnibook_console_blank_enabled = 0;

extern int (*console_blank_hook) (int);

/*
 * We would need an io_op parameter,but we are bound to the crapy
 * console_blank_hook here
 */

int omnibook_lcd_blank(int blank)
{
	int retval = 0;
	
	if ( blank_driver.io_op->backend == PIO )
		omnibook_apply_write_mask(blank_driver.io_op, blank);
	else if ( blank_driver.io_op->backend == KBC ||
		  blank_driver.io_op->backend == CDI )
		omnibook_toggle(blank_driver.io_op, blank);
	else {
		printk(O_WARN
		       "LCD console blanking is unsupported on this machine.\n");
		retval = -ENODEV;
	}
	
	return retval;
}

static int console_blank_register_hook(void)
{
	if (omnibook_console_blank_enabled == 0) {
		if (console_blank_hook == NULL) {
			console_blank_hook = omnibook_lcd_blank;
			printk(O_INFO
			        "LCD backlight turn off at console blanking is enabled.\n");

			omnibook_console_blank_enabled = 1;
		} else {
			printk(O_INFO
			        "There is a console blanking solution already registered.\n");
		}
	}
	return 0;
}

static int console_blank_unregister_hook(void)
{
	if (console_blank_hook == omnibook_lcd_blank) {
		console_blank_hook = NULL;
		printk(O_INFO "LCD backlight turn off at console blanking is disabled.\n");
		omnibook_console_blank_enabled = 0;
	} else if (console_blank_hook) {
		printk(O_WARN "You can not disable another console blanking solution.\n");
		return -EBUSY;
	} else {
		printk(O_INFO "Console blanking already disabled.\n");
		return 0;
	}
	return 0;
}

static int omnibook_console_blank_read(char *buffer,struct omnibook_operation *io_op)
{
	int len = 0;

	len +=
	    sprintf(buffer + len, "LCD console blanking is %s\n",
		    (omnibook_console_blank_enabled) ? "enabled" : "disabled");

	return len;
}

static int omnibook_console_blank_write(char *buffer,struct omnibook_operation *io_op)
{
	int retval;

	switch (*buffer) {
	case '0':
		if ((retval = console_blank_unregister_hook()))
			return retval;
		break;
	case '1':
		if ((retval = console_blank_register_hook()))
			return retval;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int __init omnibook_console_blank_init(struct omnibook_operation *io_op)
{
	return console_blank_register_hook();
}

static void __exit omnibook_console_blank_cleanup(struct omnibook_operation *io_op)
{
	console_blank_unregister_hook();
}

static struct omnibook_tbl blank_table[] __initdata = {
	{ TSM30X,	{CDI, 0, TSM100_BLANK_INDEX, 0, TSM100_LCD_OFF, TSM100_LCD_ON}}, 
	{ XE3GF|XE3GC|AMILOD|TSP10|TSM30X|TSM40, COMMAND(KBC,OMNIBOOK_KBC_CMD_LCD_OFF,OMNIBOOK_KBC_CMD_LCD_ON)},
	{ OB500|OB6000|XE2, { PIO, OB500_GPO1, OB500_GPO1, 0, -OB500_BKLT_MASK, OB500_BKLT_MASK}},
	{ OB510|OB6100,	    { PIO, OB510_GPO2, OB510_GPO2, 0, -OB510_BKLT_MASK, OB510_BKLT_MASK}},
	{ 0,}
};


static struct omnibook_feature __declared_feature blank_driver = {
	 .name = "blank",
	 .enabled = 1,
	 .read = omnibook_console_blank_read,
	 .write = omnibook_console_blank_write,
	 .init = omnibook_console_blank_init,
	 .exit = omnibook_console_blank_cleanup,
	 .ectypes = XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE2|AMILOD|TSP10|TSM30X|TSM40,
	 .tbl = blank_table,
};

module_param_named(blank, blank_driver.enabled, int, S_IRUGO);
MODULE_PARM_DESC(blank, "Use 0 to disable, 1 to enable lcd console blanking");
/* End of file */
