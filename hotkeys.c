/*
 * hotkeys.c -- code to handling Hotkey/E-Key/EasyAccess buttons
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

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include "ec.h"

/* 
 * There is no information about reading Hotkeys status
 */
static int omnibook_hotkeys_enabled = 0;

/*
 * Some laptop model have require more work to enable an Fn+button hotkeys
 */
static int omnibook_fnkeys_enabled = 0;

/*
 * Enable hotkeys sending command to the i8042
 */
static int omnibook_hotkeys_on(void)
{
	int retval;
	
	if ((retval =omnibook_kbc_command
	    (OMNIBOOK_KBC_CONTROL_CMD, OMNIBOOK_KBC_CMD_ONETOUCH_ENABLE))) {
		printk(O_ERR "Hotkeys enable command failed.\n");
		return retval;
	}
	omnibook_hotkeys_enabled = 1;
	printk(O_INFO "Enabling Hotkey buttons.\n");
	return 0;
}

/*
 * Disable hotkeys sending command to the i8042
 */
static int omnibook_hotkeys_off(void)
{
	int retval;
	
	if ((retval = omnibook_kbc_command
	    (OMNIBOOK_KBC_CONTROL_CMD, OMNIBOOK_KBC_CMD_ONETOUCH_DISABLE))) {
		printk(O_ERR "Hotkeys disable command failed.\n");
		return retval;
	}
	omnibook_hotkeys_enabled = 0;
	printk(O_INFO "Disabling Hotkey buttons.\n");
	return 0;
}

/*
 * Enable Fn+foo key writing 75h to EC index 45h
 */
static int omnibook_fnkeys_on(void)
{
	int retval;
	
	if ((retval = omnibook_cdimode_write(TSM70_FN_INDEX, TSM70_FN_ENABLE))) {
		printk(O_ERR "fnkeys enable command failed.\n");
		return retval;
	}
	omnibook_fnkeys_enabled = 1;
	printk(O_INFO "Enabling Fn keys.\n");
	return 0;
}

/*
 * FIXME Not implemented
 */
static int omnibook_fnkeys_off(void)
{
	printk(O_WARN "Disabling of Fn keys not implemented.\n");
	return 0;
}

/*
 * Power management handlers
 */
 
/*
 * omnibook_*keys_enabled = 1 means here "It was enabled prior to suspend, 
 * please reenable"
 */
static int omnibook_hotkeys_resume(void)
{
	int retval = 0;
	retval = (omnibook_hotkeys_enabled ? omnibook_hotkeys_on() : 0);
	if (omnibook_ectype & TSM30X)
		retval = (omnibook_fnkeys_enabled ? omnibook_fnkeys_on() : 0);
	return retval;
}

static int omnibook_hotkeys_suspend(void)
{
	int retval = 0;
	retval = (omnibook_hotkeys_enabled ? omnibook_hotkeys_off() : 0);
	if (omnibook_ectype & TSM30X)
		retval = (omnibook_fnkeys_enabled ? omnibook_fnkeys_off() : 0);
	return retval;
}

static int omnibook_hotkeys_enable(void)
{
	int retval = 0;

	if (!omnibook_hotkeys_enabled )
		retval = omnibook_hotkeys_on();
	if ((!omnibook_fnkeys_enabled) && ( omnibook_ectype & TSM30X ))
		retval = omnibook_fnkeys_on();

	return retval;
}

static int omnibook_hotkeys_disable(void)
{
	int retval = 0;

	if (omnibook_hotkeys_enabled )
		retval = omnibook_hotkeys_off();
	if (omnibook_fnkeys_enabled && ( omnibook_ectype & TSM30X ))
		retval = omnibook_fnkeys_off();

	return retval;
}

static int omnibook_hotkeys_read(char *buffer)
{
	int len = 0;

	len += sprintf(buffer + len, "Hotkey buttons are %s\n",
		      (omnibook_hotkeys_enabled) ? "enabled" : "disabled");

	if (omnibook_ectype & TSM30X )
		len += sprintf(buffer + len, "Fn keys are %s\n",
			    (omnibook_fnkeys_enabled) ? "enabled" : "disabled");
	return len;
}

static int omnibook_hotkeys_write(char *buffer)
{
	int retval = 0;
	switch (*buffer) {
	case '0':
		retval = omnibook_hotkeys_disable();
		break;
	case '1':
		retval = omnibook_hotkeys_enable();
		break;
	default:
		retval = -EINVAL;
	}
	return retval;
}

static int omnibook_hotkeys_init(void)
{
	int retval = 0;
	
	if ((omnibook_ectype & TSM30X) && omnibook_cdimode_init()) {
		retval = -ENODEV;
		goto out;
	}
			
	retval = omnibook_hotkeys_enable();

out:
	return retval;
}

static void omnibook_hotkeys_cleanup(void)
{
	omnibook_hotkeys_disable();
	
	if (omnibook_ectype & TSM30X)
		omnibook_cdimode_exit();
}

static struct omnibook_feature __declared_feature hotkeys_feature = {
	 .name = "hotkeys",
	 .enabled = 1,
	 .read = omnibook_hotkeys_read,
	 .write = omnibook_hotkeys_write,
	 .init = omnibook_hotkeys_init,
	 .exit = omnibook_hotkeys_cleanup,
	 .suspend = omnibook_hotkeys_suspend,
	 .resume = omnibook_hotkeys_resume,
	 .ectypes = XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|AMILOD|TSP10|TSM30X,
};

module_param_named(hotkeys, hotkeys_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(hotkeys, "Use 0 to disable, 1 to enable hotkeys handling");
/* End of file */
