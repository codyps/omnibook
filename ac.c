/*
 * ac.c -- AC adapter related functions
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

int omnibook_get_ac(void)
{
	u8 ac;
	int retval;
	/*
 	 * XE3GF
 	 * TSP10
	 * TSM30X
	 */
	if (omnibook_ectype & (XE3GF|TSP10|TSM30X) ) {
		if ((retval = omnibook_ec_read(XE3GF_ADP, &ac)))
			return retval;
		retval = (ac & XE3GF_ADP_MASK) ? 1 : 0;
	/*
	 * XE3GC
	 * AMILOD
	 */
	} else if (omnibook_ectype & (XE3GC|AMILOD) ) {	
		if ((retval = omnibook_ec_read(XE3GC_STA1, &ac)))
			return retval;
		retval = (ac & XE3GC_ADP_MASK) ? 1 : 0;
	/*
	 * OB500
	 * OB510
	 * 0B6000
	 * 0B61000
	 * XE4500
	 */
	} else if (omnibook_ectype & (OB500|OB510|OB6000|OB6100|XE4500) ) {
		if ((retval = omnibook_ec_read(OB500_STA2, &ac)))
			return retval;
		retval = (ac & OB500_ADP_MASK) ? 1 : 0;
	/*
	 * OB4150
	 */
	} else if (omnibook_ectype & OB4150 ) {
		if ((retval = omnibook_ec_read(OB4150_ADP, &ac)))
			return retval;
		retval = (ac & OB4150_ADP_MASK) ? 1 : 0;
	/*
	 * XE2
	 */
	} else if (omnibook_ectype & XE2) {
		if ((retval = omnibook_ec_read(XE2_STA1, &ac)))
			return retval;
		retval = (ac & XE2_ADP_MASK) ? 1 : 0;
	/*
	 * UNKNOWN
	 */
	} else {
		printk(KERN_INFO
		       "%s: AC adapter status monitoring is unsupported on this machine.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}
	return retval;
}

static int omnibook_ac_read(char *buffer)
{
	int len = 0;
	int ac;

	ac = omnibook_get_ac();
	if (ac < 0)
		return ac;

	len += sprintf(buffer + len, "AC %s\n", (ac) ? "on-line" : "off-line");

	return len;
}

static struct omnibook_feature __declared_feature ac_feature = {
	 .name = "ac",
	 .enabled = 1,
	 .read = omnibook_ac_read,
	 .ectypes = XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|OB4150|XE2|AMILOD|TSP10|TSM30X,
};

module_param_named(ac, ac_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(ac, "Use 0 to disable, 1 to enable AC adapter status monitoring");

/* End of file */
