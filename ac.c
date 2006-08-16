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
 * Modified by Mathieu Bérard <mathieu.berard@crans.org>, 2006
 */

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include "ec.h"

static const struct omnibook_io_operation ac_io_table[] = {
	{ XE3GF|TSP10|TSM30X, 		    EC,	XE3GF_ADP,  0, XE3GF_ADP_MASK},
	{ XE3GC|AMILOD,			    EC,	XE3GC_STA1, 0, XE3GC_ADP_MASK},
	{ OB500|OB510|OB6000|OB6100|XE4500, EC,	OB500_STA2, 0, OB500_ADP_MASK},
	{ OB4150, 			    EC,	OB4150_ADP, 0, OB4150_ADP_MASK},
	{ XE2, 				    EC,	XE2_STA1,   0, XE2_ADP_MASK},
	{ 0,}
};

static struct omnibook_io_operation *ac_io;
	
int omnibook_get_ac(void)
{
	u8 ac;
	int retval;

	retval = omnibook_io_read( ac_io, &ac);
	if (!retval)
		retval = ac ? 1 : 0;
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

static int omnibook_ac_init(void)
{
	ac_io = omnibook_io_match(ac_io_table);
	return ac_io ? 0 : -ENODEV;
}

static struct omnibook_feature __declared_feature ac_feature = {
	 .name = "ac",
	 .enabled = 1,
	 .read = omnibook_ac_read,
	 .init = omnibook_ac_init,
	 .ectypes = XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|OB4150|XE2|AMILOD|TSP10|TSM30X,
};

module_param_named(ac, ac_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(ac, "Use 0 to disable, 1 to enable AC adapter status monitoring");

/* End of file */
