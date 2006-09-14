/*
 * mutled.c -- MUTE LED control
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
 * Written by Thomas Perl <thp@perli.net>, 2006
 * Modified by Mathieu Bérard <mathieu.berard@crans.org>, 2006
 */

#include "omnibook.h"
#include "ec.h"

/* There is no information about reading MUTE LED status */
static int omnibook_muteled_enabled = 0;

static int omnibook_muteled_set(struct omnibook_operation *io_op, int status)
{
	if (omnibook_toggle(io_op, !!status)) {
		printk(O_ERR "Failed muteled %s command.\n", status ? "on" : "off");
		return -EIO;
	}
	return 0;
}

/*
 * Hardware query is unsupported, reading is unreliable.
 */
static int omnibook_muteled_read(char *buffer, struct omnibook_operation *io_op)
{
	int len = 0;

	len +=
	    sprintf(buffer + len, "Last mute LED action was an %s command.\n",
		    (omnibook_muteled_enabled) ? "on" : "off");

	return len;
}

static int omnibook_muteled_write(char *buffer, struct omnibook_operation *io_op)
{
	int cmd;

	if (*buffer == '0' || *buffer == '1') {
		cmd = *buffer - '0';
		if (!omnibook_muteled_set(io_op, cmd)) {
			omnibook_muteled_enabled = cmd;
			printk(O_INFO "Switching mute LED to %s state.\n", cmd ? "on" : "off");
		}
	} else {
		return -EINVAL;
	}
	return 0;
}

static int omnibook_muteled_resume(struct omnibook_operation *io_op)
{
	return omnibook_muteled_set(io_op, omnibook_muteled_enabled);
}

static struct omnibook_tbl muteled_table[] __initdata = {
	{XE4500, COMMAND(KBC, OMNIBOOK_KBC_CMD_MUTELED_ON, OMNIBOOK_KBC_CMD_MUTELED_OFF)},
	{0,}
};

static struct omnibook_feature __declared_feature muteled_driver = {
	.name = "muteled",
	.enabled = 1,
	.read = omnibook_muteled_read,
	.write = omnibook_muteled_write,
	.resume = omnibook_muteled_resume,
	.ectypes = XE4500,
	.tbl = muteled_table,
};

module_param_named(muteled, muteled_driver.enabled, int, S_IRUGO);
MODULE_PARM_DESC(muteled, "Use 0 to disable, 1 to enable 'Audo Mute' LED control");
