/*
 * dock.c -- docking station/port replicator support
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

static int omnibook_get_dock(void)
{
	u8 dock;
	int retval;
	/*
	 * XE3GF  
	 */
	if (omnibook_ectype & (XE3GF) ) {
		if ((retval = omnibook_ec_read(XE3GF_CSPR, &dock)))
			return retval;
		retval = (dock & XE3GF_CSPR_MASK) ? 1 : 0;
	/*
	 * OB500
	 * OB510
	 * OB6000
	 * OB6100 
	 */
	} else if (omnibook_ectype & (OB500|OB510|OB6000|OB6100) ) {	
		if ((retval = omnibook_ec_read(OB500_STA1, &dock)))
			return retval;
		retval = (dock & OB500_DCKS_MASK) ? 1 : 0;
	/*
	 * OB4150 
	 */
	} else if (omnibook_ectype & (OB4150) ) {
		if ((retval = omnibook_ec_read(OB4150_DCID, &dock)))
			return retval;
		retval = (dock) ? 1 : 0;
	} else {
		printk(KERN_INFO
		       "%s: Docking station handling is unsupported on this machine.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}

	return retval;
}

static int omnibook_dock_read(char *buffer)
{
	int len = 0;
	int dock;

	dock = omnibook_get_dock();
	if (dock < 0)
		return dock;

	len +=
	    sprintf(buffer + len, "Laptop is %s\n",
		    (dock) ? "docked" : "undocked");

	return len;
}

struct omnibook_feature dock_feature = {
	 .name = "dock",
	 .enabled = 0,
	 .read = omnibook_dock_read,
	 .ectypes = XE3GF|OB500|OB510|OB6000|OB6100|OB4150,
};

module_param_named(dock, dock_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(dock, "Use 0 to disable, 1 to enable docking station support");
/* End of file */
