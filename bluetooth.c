/*
 * wireless.c Bluetooth feature
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * Written by Mathieu Bérard <mathieu.berard@crans.org>, 2006
 *
 */

#include "omnibook.h"
#include "ec.h"

static int omnibook_bt_read(char *buffer, struct omnibook_operation *io_op)
{
	int len = 0;
	int retval;
	unsigned int state;

	if ((retval = io_op->backend->aerial_get(io_op, &state)))
		return retval;

	len +=
	    sprintf(buffer + len, "Bluetooth adapter is %s",
		    (state & BT_EX) ? "present" : "absent");
	if (state & BT_EX)
		len += sprintf(buffer + len, " and %s", (state & BT_STA) ? "enabled" : "disabled");
	len += sprintf(buffer + len, ".\n");
	return len;

}

static int omnibook_bt_write(char *buffer, struct omnibook_operation *io_op)
{
	int retval = 0;
	unsigned int state;

	if ((retval = io_op->backend->aerial_get(io_op, &state)))
		return retval;

	if (*buffer == '0')
		state &= ~BT_STA;
	else if (*buffer == '1')
		state |= BT_STA;
	else
		return -EINVAL;

	if ((retval = io_op->backend->aerial_set(io_op, state)))
		return retval;

	return retval;
}

static struct omnibook_feature bt_feature;

static int __init omnibook_bt_init(struct omnibook_operation *io_op)
{
	int retval = 0;
	unsigned int state;

/*
 *  Refuse enabling/disabling a non-existent device
 */

	if ((retval = io_op->backend->aerial_get(io_op, &state)))
		return retval;

	if (!(state & BT_EX))
		bt_feature.write = NULL;

	return retval;
}

/*
 * To avoid duplication the table is in wireless.c
 */
extern struct omnibook_tbl wireless_table[];

static struct omnibook_feature __declared_feature bt_driver = {
	.name = "bluetooth",
	.enabled = 1,
	.read = omnibook_bt_read,
	.write = omnibook_bt_write,
	.init = omnibook_bt_init,
	.ectypes = TSM30X | TSM40,
	.tbl = wireless_table,
};

module_param_named(bluetooth, bt_driver.enabled, int, S_IRUGO);
MODULE_PARM_DESC(bluetooth, "Use 0 to disable, 1 to enable bluetooth adapter control");
