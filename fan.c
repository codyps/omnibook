/*
 * fan.c -- fan status/control
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

#include <linux/delay.h>
#include "ec.h"

static int omnibook_get_fan(void)
{
	u8 fan;
	int retval;

	/*
	 * XE3GF
	 * TSP10 
	 */
	if (omnibook_ectype & (XE3GF|TSP10) ) {
		if ((retval = omnibook_ec_read(XE3GF_FSRD, &fan)))
			return retval;
		retval = fan;
	/*
	 * OB500 
	 */
	} else if (omnibook_ectype & (OB500) ) {
		if ((retval = omnibook_io_read(OB500_GPO1, &fan)))
			return retval;
		retval = (fan & OB500_FAN_OFF_MASK) ? 0 : 1;
	/*
	 * OB510 
	 */
	} else if (omnibook_ectype & (OB510) ) {
		if ((retval = omnibook_io_read(OB510_GPIO, &fan)))
			return retval;
		retval = (fan & OB510_FAN_OFF_MASK) ? 0 : 1;
	/*
	 * OB6000
	 * OB6100 
	 */
	} else if (omnibook_ectype & (OB6000|OB6100) ) {
		if ((retval = omnibook_ec_read(OB6000_STA1, &fan)))
			return retval;
		retval = (fan & OB6000_FAN_MASK) ? 1 : 0;
	/*
	 * OB4150
	 * AMILOD 
	 */
	} else if (omnibook_ectype & (OB4150|AMILOD) ) {
		if ((retval = omnibook_ec_read(OB4150_STA1, &fan)))
			return retval;
		retval = (fan & OB4150_FAN_MASK) ? 1 : 0;
	/*
	 * XE2 
	 */
	} else if (omnibook_ectype & (XE2) ) {
		if ((retval = omnibook_io_read(OB500_GPO1, &fan)))
			return retval;
		retval = (fan & XE2_FAN_MASK) ? 0 : 1;
	} else {
		printk(KERN_INFO
		       "%s: Fan status monitoring is unsupported on this machie.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}

	return retval;
}

static int omnibook_fan_on(void)
{
	u8 fan;
	int retval;

	/*
	 * XE3GF
	 * TSP10 
	 */
	if (omnibook_ectype & (XE3GF|TSP10) ) {
		if ((retval = omnibook_ec_read(XE3GF_FSRD, &fan)))
			return retval;
		if ((retval = omnibook_ec_write(XE3GF_FSRD, fan | XE3GF_FAN_ON_MASK)))
			return retval;
	/*
	 * OB500 
	 */
	} else if (omnibook_ectype & (OB500) ) {
	
		if ((retval = omnibook_io_read(OB500_GPO1, &fan)))
			return retval;
		if ((retval =
		     omnibook_io_write(OB500_GPO1, fan & ~OB500_FAN_ON_MASK)))
			return retval;
	/*
	 * OB510 
	 */
	} else if (omnibook_ectype & (OB510) ) {
	
		if ((retval = omnibook_io_read(OB510_GPIO, &fan)))
			return retval;
		if ((retval =
		     omnibook_io_write(OB510_GPIO, fan & ~OB510_FAN_ON_MASK)))
			return retval;
	/*
	 * OB6000
	 * OB6100 
	 */
	} else if (omnibook_ectype & (OB6000|OB6100) ) {
	
		if ((retval = omnibook_ec_read(OB6000_STA1, &fan)))
			return retval;
		if ((retval = omnibook_ec_write(OB6000_STA1, fan | OB6000_FAN_MASK)))
			return retval;
	/*
	 * OB4150
	 * AMILOD 
	 */
	} else if (omnibook_ectype & (OB4150|AMILOD) ) {
		if ((retval = omnibook_ec_read(OB4150_STA1, &fan)))
			return retval;
		if ((retval = omnibook_ec_write(OB4150_STA1, fan | OB4150_FAN_MASK)))
			return retval;
	/*
	 * XE2 
	 */
	} else if (omnibook_ectype & (XE2) ) {
	
		if ((retval = omnibook_io_read(OB500_GPO1, &fan)))
			return retval;
		if ((retval =
		     omnibook_io_write(OB500_GPO1, fan & ~XE2_FAN_MASK)))
			return retval;
	} else {
		printk(KERN_INFO
		       "%s: Direct fan control is unsupported on this machie.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}

	return retval;
}

static int omnibook_fan_off(void)
{
	u8 fan;
	int retval;

	/*
	 * XE3GF
	 * TSP10 
	 */
	if (omnibook_ectype & (XE3GF|TSP10) ) {
		u8 fot, temp;
		if ((retval = omnibook_ec_read(XE3GF_FSRD, &fan)))
			return retval;

		/* fan is already off */
		if (!fan)
			return 0;

		/* now we set FOT to current temp, then reset to initial value */
		if ((retval = omnibook_ec_read(XE3GF_FOT, &fot)))
			return retval;
		if ((retval = omnibook_ec_read(XE3GF_CTMP, &temp)))
			return retval;

		do {
			omnibook_ec_write(XE3GF_FOT, temp);
			mdelay(1);
		} while (omnibook_get_fan() != 0);

		omnibook_ec_write(XE3GF_FOT, fot);
	/*
	 * OB500 
	 */
	} else if (omnibook_ectype & (OB500) ) {
		if ((retval = omnibook_io_read(OB500_GPO1, &fan)))
			return retval;
		if ((retval =
		     omnibook_io_write(OB500_GPO1, fan | OB500_FAN_OFF_MASK)))
			return retval;
	/*
	 * OB510 
	 */
	} else if (omnibook_ectype & (OB510) ) {
		if ((retval = omnibook_io_read(OB510_GPIO, &fan)))
			return retval;
		if ((retval =
		     omnibook_io_write(OB510_GPIO, fan | OB510_FAN_OFF_MASK)))
			return retval;
	/*
	 * OB6000
	 * OB6100 
	 */
	} else if (omnibook_ectype & (OB6000|OB6100) ) {
		if ((retval = omnibook_ec_read(OB6000_STA1, &fan)))
			return retval;
		if ((retval = omnibook_ec_write(OB6000_STA1, fan & ~OB6000_FAN_MASK)))
			return retval;
	/*
	 * OB4150
	 * AMILOD 
	 */
	} else if (omnibook_ectype & (OB4150|AMILOD) ) {
		if ((retval = omnibook_ec_read(OB4150_STA1, &fan)))
			return retval;
		if ((retval = omnibook_ec_write(OB4150_STA1, fan & ~OB4150_FAN_MASK)))
			return retval;
	/*
	 * XE2 
	 */
	} else if (omnibook_ectype & (XE2) ) {
		if ((retval = omnibook_io_read(OB500_GPO1, &fan)))
			return retval;
		if ((retval =
		     omnibook_io_write(OB500_GPO1, fan | XE2_FAN_MASK)))
			return retval;
	} else {
		printk(KERN_INFO
		       "%s: Direct fan control is unsupported on this machie.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}

	return retval;
}

static int omnibook_fan_read(char *buffer)
{
	int fan;
	int len = 0;
	char *str;

	fan = omnibook_get_fan();
	if (fan < 0)
		return fan;
	str = (fan) ? "on" : "off";

	if (fan > 1)
		len += sprintf(buffer + len, "Fan is %s (level %d)\n", str, fan);
	else
		len += sprintf(buffer + len, "Fan is %s\n", str);

	return len;
}

static int omnibook_fan_write(char *buffer)
{
	int retval;

	switch (*buffer) {
	case '0':
		if ((retval = omnibook_fan_off()))
			return retval;
		break;
	case '1':
		if ((retval = omnibook_fan_on()))
			return retval;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

struct omnibook_feature fan_feature;

static int __init omnibook_fan_init(void)
{
	/*
	 * OB4150
	 * XE2
	 * AMILOD
	 * They only support fan reading 
	 */
	if (omnibook_ectype & (OB4150|XE2|AMILOD) )
		fan_feature.write = NULL;
	return 0;
}

struct omnibook_feature fan_feature = {
	 .name = "fan",
	 .enabled = 1,
	 .read = omnibook_fan_read,
	 .write = omnibook_fan_write,
	 .init = omnibook_fan_init,
	 .ectypes = XE3GF|OB500|OB510|OB6000|OB6100|OB4150|XE2|AMILOD|TSP10,
};

module_param_named(fan, fan_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(fan, "Use 0 to disable, 1 to enable fan status monitor and control");
/* End of file */
