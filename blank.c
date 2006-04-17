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
 */

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include "ec.h"

int omnibook_console_blank_enabled = 0;

extern int (*console_blank_hook) (int);

int omnibook_lcd_blank(int blank)
{
	int retval = 0;
	u8 cmd;
	/*
	 * XE3GF
	 * XE3GC
	 * AMILOD
	 * TSP10
	 * TSM30X
	 * TSM40
	 */
	if (omnibook_ectype & (XE3GF|XE3GC|AMILOD|TSP10|TSM30X|TSM40) ) {
		cmd =
		    blank ? OMNIBOOK_KBC_CMD_LCD_OFF : OMNIBOOK_KBC_CMD_LCD_ON;
		if ((retval =
		     omnibook_kbc_command(OMNIBOOK_KBC_CONTROL_CMD, cmd)))
			return retval;
	/*
	 * OB500
	 * OB6000
	 * XE2
	 */
	} else if (omnibook_ectype & (OB500|OB6000|XE2) ) {
		if ((retval = omnibook_io_read(OB500_GPO1, &cmd)))
			return retval;
		cmd = blank ? cmd & ~OB500_BKLT_MASK : cmd | OB500_BKLT_MASK;
		if ((retval = omnibook_io_write(OB500_GPO1, cmd)))
			return retval;
	/*
	 * OB510
	 * 0B61000
	 */
	} else if (omnibook_ectype & (OB510|OB6100) ) {
		if ((retval = omnibook_io_read(OB510_GPO2, &cmd)))
			return retval;
		cmd = blank ? cmd & ~OB510_BKLT_MASK : cmd | OB510_BKLT_MASK;
		if ((retval = omnibook_io_write(OB510_GPO2, cmd)))
			return retval;
	/*
	 * UNKNOWN
	 */
	} else {
		printk(KERN_INFO
		       "%s: LCD console blanking is unsupported on this machine.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}
	return retval;
}

static int omnibook_console_blank_enable(void)
{
	if (omnibook_console_blank_enabled == 0) {
		if (console_blank_hook == NULL) {
			console_blank_hook = omnibook_lcd_blank;
			printk(KERN_INFO
			       "%s: LCD backlight turn off at console blanking is enabled.\n",
			       OMNIBOOK_MODULE_NAME);
			omnibook_console_blank_enabled = 1;
		} else {
			printk(KERN_INFO
			       "%s: There is a console blanking solution already registered.\n",
			       OMNIBOOK_MODULE_NAME);
		}
	}
	return 0;
}

static int omnibook_console_blank_disable(void)
{
	if (console_blank_hook == omnibook_lcd_blank) {
		console_blank_hook = NULL;
		printk(KERN_INFO
		       "%s: LCD backlight turn off at console blanking is disabled.\n",
		       OMNIBOOK_MODULE_NAME);
		omnibook_console_blank_enabled = 0;
	} else if (console_blank_hook) {
		printk(KERN_WARNING
		       "%s: You can not disable another console blanking solution.\n",
		       OMNIBOOK_MODULE_NAME);
		return -EBUSY;
	} else {
		printk(KERN_INFO "%s: Console blanking already disabled.\n",
		       OMNIBOOK_MODULE_NAME);
		return 0;
	}
	return 0;
}

static int omnibook_console_blank_read(char *buffer)
{
	int len = 0;

	len +=
	    sprintf(buffer + len, "LCD console blanking is %s\n",
		    (omnibook_console_blank_enabled) ? "enabled" : "disabled");

	return len;
}

static int omnibook_console_blank_write(char *buffer)
{
	int retval;

	switch (*buffer) {
	case '0':
		if ((retval = omnibook_console_blank_disable()))
			return retval;
		break;
	case '1':
		if ((retval = omnibook_console_blank_enable()))
			return retval;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int __init omnibook_console_blank_init(void)
{

	int retval;

	if ((retval = omnibook_console_blank_enable()))
		return retval;

	return 0;
}

static void __exit omnibook_console_blank_cleanup(void)
{
	omnibook_console_blank_disable();
}

struct omnibook_feature blank_feature = {
	 .name = "blank",
	 .enabled = 1,
	 .read = omnibook_console_blank_read,
	 .write = omnibook_console_blank_write,
	 .init = omnibook_console_blank_init,
	 .exit = omnibook_console_blank_cleanup,
	 .ectypes = XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE2|AMILOD|TSP10|TSM30X|TSM40,
};

module_param_named(blank, blank_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(blank, "Use 0 to disable, 1 to enable lcd console blanking");
/* End of file */
