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
 * Modified by Mathieu Bérard <mathieu.berard@crans.org>, 2006
 */

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include "ec.h"

static const struct omnibook_io_operation dock_io_table[] = {
	{ XE3GF,		     EC, XE3GF_CSPR, 0, XE3GF_CSPR_MASK},
	{ OB500|OB510|OB6000|OB6100, EC, OB500_STA1, 0, OB500_DCKS_MASK},
	{ OB4150,		     EC, OB4150_DCID, 0, 0},	
	{ 0,}
};

static struct omnibook_io_operation *dock_io;

static int omnibook_dock_read(char *buffer)
{
	int len = 0;
	u8 dock;
	int retval;
	
	if ((retval = omnibook_io_read(dock_io, &dock)))
		return retval;
	
	len +=
	    sprintf(buffer + len, "Laptop is %s\n",
		    (dock) ? "docked" : "undocked");

	return len;
}

static int omnibook_dock_init(void)
{
	dock_io = omnibook_io_match(dock_io_table);
	return dock_io ? 0 : -ENODEV;
}

static struct omnibook_feature __declared_feature dock_feature = {
	 .name = "dock",
	 .enabled = 0,
	 .read = omnibook_dock_read,
	 .init = omnibook_dock_init,
	 .ectypes = XE3GF|OB500|OB510|OB6000|OB6100|OB4150,
};

module_param_named(dock, dock_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(dock, "Use 0 to disable, 1 to enable docking station support");
/* End of file */
