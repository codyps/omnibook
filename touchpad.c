/*
 * touchpad.c -- enable/disable touchpad
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
#include "ec.h"

/* Touchpad is assumed to be enabled by default */
static int omnibook_touchpad_enabled = 1;

static int omnibook_touchpad_set(struct omnibook_operation *io_op, int status)
{
	int retval = 0;
	if ((retval = omnibook_toggle(io_op, !!status))) {
		printk(O_ERR "Failed touchpad %sable command.\n", status ? "en" : "dis");
	}
	return retval;
}

/*
 * Power management handlers: redisable touchpad on resume (if requested)
 */
static int omnibook_touchpad_resume(struct omnibook_operation *io_op)
{
	int retval;
	retval = (omnibook_touchpad_enabled ? 0 : omnibook_touchpad_set(io_op, 0));
	return retval;
}

/*
 * Hardware query is unsupported, so reading is unreliable.
 */
static int omnibook_touchpad_read(char *buffer, struct omnibook_operation *io_op)
{
	int len = 0;

	len +=
	    sprintf(buffer + len, "Last touchpad action was an %s command.\n",
		    (omnibook_touchpad_enabled) ? "enable" : "disable");

	return len;
}

static int omnibook_touchpad_write(char *buffer, struct omnibook_operation *io_op)
{
	int cmd;

	if (*buffer == '0' || *buffer == '1') {
		cmd = *buffer - '0';
		if (!omnibook_touchpad_set(io_op, cmd)) {
			omnibook_touchpad_enabled = cmd;
			printk(O_INFO "%sabling touchpad.\n", cmd ? "En" : "Dis");
		}
	} else {
		return -EINVAL;
	}
	return 0;
}

/*
 * Reenable touchpad upon exit
 */
static void __exit omnibook_touchpad_cleanup(struct omnibook_operation *io_op)
{
	omnibook_touchpad_set(io_op, 1);
	printk(O_INFO "Enabling touchpad.\n");
}

static struct omnibook_tbl touchpad_table[] __initdata = {
	{XE3GF | XE3GC | TSP10,
	 COMMAND(KBC, OMNIBOOK_KBC_CMD_TOUCHPAD_ENABLE, OMNIBOOK_KBC_CMD_TOUCHPAD_DISABLE)},
	{TSM30X, {CDI, 0, TSM70_FN_INDEX, 0, TSM70_TOUCHPAD_ON, TSM70_TOUCHPAD_OFF}},
	{0,}
};

static struct omnibook_feature __declared_feature touchpad_driver = {
	.name = "touchpad",
	.enabled = 1,
	.read = omnibook_touchpad_read,
	.write = omnibook_touchpad_write,
	.exit = omnibook_touchpad_cleanup,
	.resume = omnibook_touchpad_resume,
	.ectypes = XE3GF | XE3GC | TSP10 | TSM30X,
	.tbl = touchpad_table,
};

module_param_named(touchpad, touchpad_driver.enabled, int, S_IRUGO);
MODULE_PARM_DESC(touchpad, "Use 0 to disable, 1 to enable touchpad handling");

/* End of file */
