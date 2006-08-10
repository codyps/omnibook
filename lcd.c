/*
 * lcd.c -- LCD brightness and on/off
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
 * Written by Maciek Górniak <mago@acn.waw.pl>, 2002
 * Modified by Soós Péter <sp@osb.hu>, 2002-2004
 */

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include <linux/err.h>

#ifdef CONFIG_OMNIBOOK_BACKLIGHT
#include <linux/backlight.h>
#endif

#include "ec.h"

static int omnibook_max_brightness;

#ifdef CONFIG_OMNIBOOK_BACKLIGHT

static struct backlight_device *omnibook_backlight_device;

static int omnibook_get_backlight(struct backlight_device *bd);
static int omnibook_set_backlight(struct backlight_device *bd);

static struct backlight_properties omnibookbl_data = {
	.owner = THIS_MODULE,
	.get_brightness = omnibook_get_backlight,
	.update_status = omnibook_set_backlight,
};

#endif

static int omnibook_get_lcd_brightness(void)
{
	int retval = 0;
	u8 brgt;

	/*
	 * XE3GF
	 * TSP10
	 * TSM30X
	 * TSM40 
	 */
	if (omnibook_ectype & (XE3GF|TSP10|TSM30X|TSM40) ) {
		if ((retval = omnibook_ec_read(XE3GF_BRTS, &brgt)))
			return retval;
		retval = brgt &= XE3GF_BRTS_MASK;
	/*
	 * XE3GC 
	 */
	} else if (omnibook_ectype & (XE3GC) ) {
	
		if ((retval = omnibook_ec_read(XE3GC_BTVL, &brgt)))
			return retval;
		retval = brgt &= XE3GC_BTVL_MASK;
	/*
	 * AMILOD 
	 */
	} else if (omnibook_ectype & (AMILOD) ) {
	
		if ((retval = omnibook_ec_read(AMILOD_CBRG, &brgt)))
			return retval;
		retval = brgt &= AMILOD_CBRG_MASK;
	} else {
		printk(KERN_INFO
		       "%s: LCD brightness handling is unsupported on this machine.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}

	return retval;
}

static int omnibook_set_lcd_brightness(int brgt)
{
	int retval = 0;

	brgt =
	    (brgt > omnibook_max_brightness) ? omnibook_max_brightness : brgt;

	/*
	 * XE3GF
	 * TSP10
	 * TSM30X
	 * TSM40 
	 */
	if (omnibook_ectype & (XE3GF|TSP10|TSM30X|TSM40) ) {
		if ((retval = omnibook_ec_write(XE3GF_BRTS, brgt)))
			return retval;
	/*
	 * XE3GC 
	 */
	} else if (omnibook_ectype & (XE3GC) ) {
		if ((retval = omnibook_ec_write(XE3GC_BTVL, brgt)))
			return retval;
	/*
	 * AMILOD 
	 */
	} else if (omnibook_ectype & (AMILOD) ) {
		if ((retval = omnibook_ec_write(AMILOD_CBRG, brgt)))
			return retval;
	/*
 	 * TSA105
 	 */
	} else if (omnibook_ectype & (TSA105) ) {
                if ((retval = omnibook_ec_write(A105_BNDT, brgt)))
                        return retval;
	} else {
		printk(KERN_INFO
		       "%s: LCD brightness handling is unsupported on this machine.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}
	return retval;
}

#ifdef CONFIG_OMNIBOOK_BACKLIGHT
static int omnibook_get_backlight(struct backlight_device *bd)
{
	return omnibook_get_lcd_brightness();
}

static int omnibook_set_backlight(struct backlight_device *bd)
{
	int intensity = bd->props->brightness;
	return omnibook_set_lcd_brightness(intensity);
}
#endif

static int omnibook_brightness_read(char *buffer)
{
	int len = 0;
	int brgt;

	brgt = omnibook_get_lcd_brightness();
	if (brgt < 0)
		return brgt;

	len += sprintf(buffer + len, "LCD brightness: %2d\n", brgt);

	return len;
}

static int omnibook_brightness_write(char *buffer)
{
	int brgt = 0;
	char *endp;

	if (strncmp(buffer, "off", 3) == 0)
		omnibook_lcd_blank(1);
	else if (strncmp(buffer, "on", 2) == 0)
		omnibook_lcd_blank(0);
	else {
		brgt = simple_strtol(buffer, &endp, 10);
		if ((endp == buffer) || (brgt < 0) || (brgt > omnibook_max_brightness))
			return -EINVAL;
		else
			omnibook_set_lcd_brightness(brgt);
	}
	return 0;
}

static int omnibook_brightness_init(void)
{
	/*
	 * FIXME: What is exactly de max value for each model ?
	 * I know that it's 7 for the TSM30X, TSM40 and TSA105
	 * and previous versions of this driver assumed it was 10 for
	 * all models.
	 * 
	 * TSM30X
	 * TSM40
	 * TSA105
	 */
	if (omnibook_ectype & (TSM30X|TSM40|TSA105) )
		omnibook_max_brightness = 7;
	else
		omnibook_max_brightness = 10;
		
	printk(KERN_INFO "%s: LCD brightness is between 0 and %i.\n",
	       OMNIBOOK_MODULE_NAME, omnibook_max_brightness);

#ifdef CONFIG_OMNIBOOK_BACKLIGHT
	omnibookbl_data.max_brightness = omnibook_max_brightness,
	    omnibook_backlight_device =
	    backlight_device_register(OMNIBOOK_MODULE_NAME, NULL, &omnibookbl_data);
	if (IS_ERR(omnibook_backlight_device)) {
		printk(KERN_ERR "%s: Unable to register as backlight device.\n",
		       OMNIBOOK_MODULE_NAME);
		return -ENODEV;
	}
#endif
	return 0;
}

static void omnibook_brightness_cleanup(void)
{
#ifdef CONFIG_OMNIBOOK_BACKLIGHT
	backlight_device_unregister(omnibook_backlight_device);
#endif
}

static struct omnibook_feature __declared_feature lcd_feature = {
	 .name = "lcd",
	 .enabled = 1,
	 .read = omnibook_brightness_read,
	 .write = omnibook_brightness_write,
	 .init = omnibook_brightness_init,
	 .exit = omnibook_brightness_cleanup,
	 .ectypes = XE3GF|XE3GC|AMILOD|TSP10|TSM30X|TSM40|TSA105,
};

module_param_named(lcd, lcd_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(lcd, "Use 0 to disable, 1 to enable to LCD brightness support");

/* End of file */
