/*
 * onetouch.c -- code to handling OneTouch buttons
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

/* There is no information about reading OneTouch status */
int omnibook_onetouch_enabled = 0;

static int omnibook_onetouch_on(void)
{
	if (omnibook_kbc_command
	    (OMNIBOOK_KBC_CONTROL_CMD, OMNIBOOK_KBC_CMD_ONETOUCH_ENABLE)) {
		printk(KERN_ERR "%s: failed OneTouch enable command.\n",
		       OMNIBOOK_MODULE_NAME);
		return -EIO;
	}
	return 0;
}

static int omnibook_onetouch_off(void)
{
	if (omnibook_kbc_command
	    (OMNIBOOK_KBC_CONTROL_CMD, OMNIBOOK_KBC_CMD_ONETOUCH_DISABLE)) {
		printk(KERN_ERR "%s: failed OneTouch disable command.\n",
		       OMNIBOOK_MODULE_NAME);
		return -EIO;
	}
	return 0;
}

/*
 * Power management handlers
 */
static int omnibook_onetouch_resume(void)
{
	int retval;
	retval = (omnibook_onetouch_enabled ? omnibook_onetouch_on() : 0);
	return retval;
}

static int omnibook_onetouch_suspend(void)
{
	int retval;
	retval = (omnibook_onetouch_enabled ? omnibook_onetouch_off() : 0);
	return retval;
}

static int omnibook_onetouch_enable(void)
{
	/*
	 * XE3GF
	 * XE3GC
	 * OB500
	 * OB510
	 * OB6000
	 * OB6100
	 * XE4500
	 * AMILOD
	 * TSP10
	 * TSM30X
	 */
	if (omnibook_ectype & (XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|AMILOD|TSP10|TSM30X) ) {
		if (!omnibook_onetouch_enabled) {
			if (omnibook_onetouch_on())
				return -EIO;
			omnibook_onetouch_enabled = 1;
			printk(KERN_INFO
			       "%s: OneTouch buttons (if any) are enabled.\n",
			       OMNIBOOK_MODULE_NAME);
		}
	} else {
		omnibook_onetouch_enabled = 0;
		return -ENODEV;
	}
	return 0;
}

static int omnibook_onetouch_disable(void)
{
	/*
	 * XE3GF
	 * XE3GC
	 * OB500
	 * OB510
	 * OB6000
	 * OB6100
	 * XE4500
	 * AMILOD
	 * TSP10
	 * TSM30X
	 */
	if (omnibook_ectype & (XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|AMILOD|TSP10|TSM30X) ) {
		if (omnibook_onetouch_enabled) {
			if (omnibook_onetouch_off()) {
				return -EIO;
			}
			omnibook_onetouch_enabled = 0;
			printk(KERN_INFO
			       "%s: OneTouch buttons (if any) are disabled.\n",
			       OMNIBOOK_MODULE_NAME);
		}
	} else {
		omnibook_onetouch_enabled = 0;
		return -ENODEV;
	}
	return 0;
}

static int omnibook_onetouch_read(char *buffer)
{
	int len = 0;

	len +=
	    sprintf(buffer + len, "OneTouch buttons are %s\n",
		    (omnibook_onetouch_enabled) ? "enabled" : "disabled");

	return len;
}

static int omnibook_onetouch_write(char *buffer)
{
	switch (*buffer) {
	case '0':
		omnibook_onetouch_disable();
		break;
	case '1':
		omnibook_onetouch_enable();
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int omnibook_onetouch_init(void)
{
	int retval;

	if ((retval = omnibook_onetouch_enable()))
		return retval;
	return 0;
}

static void omnibook_onetouch_cleanup(void)
{
	omnibook_onetouch_disable();
}

struct omnibook_feature onetouch_feature = {
	 .name = "onetouch",
	 .enabled = 1,
	 .read = omnibook_onetouch_read,
	 .write = omnibook_onetouch_write,
	 .init = omnibook_onetouch_init,
	 .exit = omnibook_onetouch_cleanup,
	 .suspend = omnibook_onetouch_suspend,
	 .resume = omnibook_onetouch_resume,
	 .ectypes = XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|AMILOD|TSP10|TSM30X,
};

module_param_named(onetouch, onetouch_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(onetouch, "Use 0 to disable, 1 to enable onetouch handling");
/* End of file */
