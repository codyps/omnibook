/*
 * display.c -- external display related functions
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

static int omnibook_get_display(void)
{
	int retval = 0;
	u8 sta;
	
	/*
	 * XE3GF
	 * TSP10
	 * TSM30X
	 * TSM40
	 */
	if (omnibook_ectype & (XE3GF|TSP10|TSM30X|TSM40) ) {
		if ((retval = omnibook_ec_read(XE3GF_STA1, &sta)))
			return retval;
		retval = (sta & XE3GF_SHDD_MASK) ? 1 : 0;
	/*
	 * XE3GC
	 */
	} else if (omnibook_ectype & XE3GC ) {
		if ((retval = omnibook_ec_read(XE3GC_STA1, &sta)))
			return retval;
		retval = (sta & XE3GC_CRTI_MASK) ? 1 : 0;
	/*
	 * OB500
	 * OB510
	 * OB6000
	 * OB6100
	 * XE4500
	 */
	 } else if (omnibook_ectype & (OB500|OB510|OB6000|OB6100|XE4500) ) {
		if ((retval = omnibook_ec_read(OB500_STA1, &sta)))
			return retval;
		retval = (sta & OB500_CRTS_MASK) ? 1 : 0;
	/*
	 * OB4150
	 */
	} else if (omnibook_ectype & OB4150 ) {
		if ((retval = omnibook_ec_read(OB4150_STA2, &sta)))
			return retval;
		retval = (sta & OB4150_CRST_MASK) ? 1 : 0;
	/*
	 * UNKNOWN
	 */
	} else {
		printk(KERN_INFO
		       "%s: External display status monitoring is unsupported on this machine.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}
	return retval;
}

static int omnibook_display_read(char *buffer)
{
	int len = 0;
	int display;

	display = omnibook_get_display();
	if (display < 0)
		return display;

	len +=
	    sprintf(buffer + len, "External display is %s\n",
		    (display) ? "present" : "not present");

	return len;
}

static struct omnibook_feature __declared_feature display_feature = {
	 .name = "display",
	 .enabled = 1,
	 .read = omnibook_display_read,
	 .ectypes = XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|OB4150|TSP10|TSM30X|TSM40,
};

module_param_named(display, display_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(display, "Use 0 to disable, 1 to enable display status handling");
/* End of file */
