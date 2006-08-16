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
 * Modified by Mathieu Bérard <mathieu.berard@crans.org>, 2006
 */

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include "ec.h"

static const struct omnibook_io_operation display_io_table[] = {
	{ XE3GF|TSP10|TSM30X|TSM40,	    EC, XE3GF_STA1, 0, XE3GF_SHDD_MASK},
	{ XE3GC,			    EC, XE3GC_STA1, 0, XE3GC_CRTI_MASK},
	{ OB500|OB510|OB6000|OB6100|XE4500, EC, OB500_STA1, 0, OB500_CRTS_MASK},
	{ OB4150,			    EC, OB4150_STA2, 0, OB4150_CRST_MASK},
	{ 0,}
};

static struct omnibook_io_operation *display_io;

static int omnibook_display_read(char *buffer)
{
	int len = 0;
	int retval;
	u8 sta;
	
	if ((retval = omnibook_io_read(display_io, &sta)))
		return retval;
	len +=
	    sprintf(buffer + len, "External display is %s\n",
		    (sta) ? "present" : "not present");

	return len;
}

static int omnibook_display_init(void)
{
	display_io = omnibook_io_match(display_io_table);
	return display_io ? 0 : -ENODEV;
}
	

static struct omnibook_feature __declared_feature display_feature = {
	 .name = "display",
	 .enabled = 1,
	 .read = omnibook_display_read,
	 .init = omnibook_display_init,
	 .ectypes = XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|OB4150|TSP10|TSM30X|TSM40,
};

module_param_named(display, display_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(display, "Use 0 to disable, 1 to enable display status handling");
/* End of file */
