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
 */

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include "ec.h"

/* Touchpad is enabled by default */
int omnibook_touchpad_enabled = 1;

static int omnibook_touchpad_on(void)
{
	if (omnibook_kbc_command
	    (OMNIBOOK_KBC_CONTROL_CMD, OMNIBOOK_KBC_CMD_TOUCHPAD_ENABLE)) {
		printk(KERN_ERR "%s: failed touchpad enable command.\n",
		       OMNIBOOK_MODULE_NAME);
		return -EIO;
	}
	return 0;
}

static int omnibook_touchpad_off(void)
{
	if (omnibook_kbc_command
	    (OMNIBOOK_KBC_CONTROL_CMD, OMNIBOOK_KBC_CMD_TOUCHPAD_DISABLE)) {
		printk(KERN_ERR "%s: failed touchpad disable command.\n",
		       OMNIBOOK_MODULE_NAME);
		return -EIO;
	}
	return 0;
}

/*
 * Power management handlers: redisable touchpad on resume
 */
int omnibook_touchpad_resume(void)
{
	int retval;
	retval = (omnibook_touchpad_enabled ? 0 : omnibook_touchpad_off());
	return retval;
}

static int omnibook_touchpad_enable(void)
{
	/*
	 * XE3GF
	 * XE3GC
	 * TSP10 
	 */
	if (omnibook_ectype & (XE3GF|XE3GC|TSP10) ) {
	
		if (!omnibook_touchpad_enabled) {
			if (omnibook_touchpad_on())
				return -EIO;
			omnibook_touchpad_enabled = 1;
			printk(KERN_INFO "%s: Touchpad is enabled.\n",
			       OMNIBOOK_MODULE_NAME);
		}
	/*
	 * These models have stickpointer, not touchpad:
	 * OB500
	 * OB510
	 */
	} else if (omnibook_ectype & (OB500|OB510) ) {
	
		omnibook_touchpad_enabled = 0;
		return -ENODEV;
	} else {
		omnibook_touchpad_enabled = 1;
		return -ENODEV;
	}
	return 0;
}

static int omnibook_touchpad_disable(void)
{
	/*
	 * XE3GF
	 * XE3GC
	 * TSP10 
	 */
	if (omnibook_ectype & (XE3GF|XE3GC|TSP10) ) {
		if (omnibook_touchpad_enabled) {
			if (omnibook_touchpad_off()) {
				return -EIO;
			}
			omnibook_touchpad_enabled = 0;
			printk(KERN_INFO "%s: Touchpad is disabled.\n",
			       OMNIBOOK_MODULE_NAME);
		}
	/*
	 * These models have stickpointer, not touchpad:
	 * OB500
	 * OB510
	 */
	} else if (omnibook_ectype & (OB500|OB510) ) {
		omnibook_touchpad_enabled = 0;
		return -ENODEV;
	} else {
		omnibook_touchpad_enabled = 1;
		return -ENODEV;
	}
	return 0;
}

static int omnibook_touchpad_read(char *buffer)
{
	int len = 0;

	len +=
	    sprintf(buffer + len, "Touchpad is %s\n",
		    (omnibook_touchpad_enabled) ? "enabled" : "disabled");

	return len;
}

static int omnibook_touchpad_write(char *buffer)
{
	switch (*buffer) {
	case '0':
		omnibook_touchpad_disable();
		break;
	case '1':
		omnibook_touchpad_enable();
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void omnibook_touchpad_cleanup(void)
{
	omnibook_touchpad_enable();
}

struct omnibook_feature touchpad_feature = {
	 .name = "touchpad",
	 .enabled = 1,
	 .read = omnibook_touchpad_read,
	 .write = omnibook_touchpad_write,
	 .exit = omnibook_touchpad_cleanup,
	 .resume = omnibook_touchpad_resume,
	 .ectypes = XE3GF|XE3GC|TSP10,
};

module_param_named(touchpad, touchpad_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(touchpad, "Use 0 to disable, 1 to enable touchpad handling");

/* End of file */
