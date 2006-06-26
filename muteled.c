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
 */

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include "ec.h"

/* There is no information about reading MUTE LED status */
static int omnibook_muteled_enabled = 0;

static int omnibook_muteled_on(void)
{
	if (omnibook_kbc_command(OMNIBOOK_KBC_CONTROL_CMD, OMNIBOOK_KBC_CMD_MUTELED_ON)) {
		printk(KERN_ERR "%s: failed muteled on command.\n", OMNIBOOK_MODULE_NAME);
		return -EIO;
	}
        omnibook_muteled_enabled = 1;
	return 0;
}

static int omnibook_muteled_off(void)
{
	if (omnibook_kbc_command(OMNIBOOK_KBC_CONTROL_CMD, OMNIBOOK_KBC_CMD_MUTELED_OFF)) {
		printk(KERN_ERR "%s: failed muteled off command.\n", OMNIBOOK_MODULE_NAME);
		return -EIO;
	}
        omnibook_muteled_enabled = 0;
	return 0;
}


static int omnibook_muteled_disable(void)
{
        omnibook_muteled_off();
        return 0;
}

static int omnibook_muteled_enable(void)
{
        omnibook_muteled_on();
        return 0;
}

static int omnibook_muteled_read(char *buffer)
{
	int len = 0;

	len += sprintf(buffer+len, "The mute LED is %s\n", (omnibook_muteled_enabled) ? "on" : "off");

	return len;
}

static int omnibook_muteled_write(char *buffer)
{
	int retval;	
	
	switch (*buffer) {
	case '0':
		if ((retval = omnibook_muteled_disable()));
			return retval;
		break;
	case '1':
		if ((retval = omnibook_muteled_enable()));
			return retval;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}


static struct omnibook_feature __declared_feature muteled_feature = {
	 .name = "muteled",
	 .enabled = 1,
	 .read = omnibook_muteled_read,
	 .write = omnibook_muteled_write,
	 .ectypes = XE4500,
};

module_param_named(muteled, muteled_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(muteled, "Use 0 to disable, 1 to enable 'Audo Mute' LED control");

