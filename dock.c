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

#include "omnibook.h"

#include "ec.h"

static int omnibook_dock_read(char *buffer,struct omnibook_operation *io_op)
{
	int len = 0;
	u8 dock;
	int retval;
	
	if ((retval = io_op->backend->byte_read(io_op,&dock)))
		return retval;
	
	len +=
	    sprintf(buffer + len, "Laptop is %s\n",
		    (dock) ? "docked" : "undocked");

	return len;
}

static struct omnibook_tbl dock_table[] __initdata = {
	{ XE3GF,		     SIMPLE_BYTE(EC,XE3GF_CSPR,XE3GF_CSPR_MASK)},
	{ OB500|OB510|OB6000|OB6100, SIMPLE_BYTE(EC,OB500_STA1,OB500_DCKS_MASK)},
	{ OB4150,		     SIMPLE_BYTE(EC,OB4150_DCID,0)},	
	{ 0,}
};


static struct omnibook_feature __declared_feature dock_driver = {
	 .name = "dock",
	 .enabled = 0,
	 .read = omnibook_dock_read,
	 .ectypes = XE3GF|OB500|OB510|OB6000|OB6100|OB4150,
	 .tbl = dock_table,
};

module_param_named(dock, dock_driver.enabled, int, S_IRUGO);
MODULE_PARM_DESC(dock, "Use 0 to disable, 1 to enable docking station support");
/* End of file */
