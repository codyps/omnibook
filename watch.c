/*
 *  watch.c - Watch a specific ioport/iomem/ec register
 *
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
 * Written by Mathieu Bérard <mathieu.berard@crans.org>, 2006
 */

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include "ec.h"

enum {
	BAD,
	EC,
	PIO,
	MMIO,
};

static u32 addr = 0;
static int addr_type = BAD;

static int watch_read(char *buffer)
{
	int len = 0;
	u8 v;
	
	switch (addr_type) {
	case BAD:
		len += sprintf(buffer + len, "Please first write to this file and address to watch\n");
		len += sprintf(buffer + len, "ex: 'TYPE 0x2f' where TYPE={EC,PIO,MMIO}\n");
		return len;
	case EC:
		omnibook_ec_read((u8) addr, &v);
		break;
	case PIO:
		omnibook_io_read(addr, &v);
		break;
	case MMIO:
		omnibook_mem_read(addr, &v);
		break;
	}
	len += sprintf(buffer + len, "%02x\n",v );
	
	return len;
}

static int watch_write(char *buffer)
{
	int retval;
	u32 value;
	char type[5];

	retval = sscanf(buffer, "%4s%x", type, &value);
	
	if (retval != 2)
		return -EINVAL;	

	if (strncmp(type,"EC",2) == 0)
		addr_type = EC;
	else if	(strncmp(type,"PIO",3) == 0)
		addr_type = PIO;
	else if (strncmp(type,"MMIO",4) == 0)
		addr_type = MMIO;
	else
		return -EINVAL;
	
 	if  ((value > 0xff) && (addr_type == EC))
		return -EINVAL;
	else if ((value > 0xffff) && (addr_type == PIO))
		return -EINVAL;
	else
		addr = value;

	return 0;
}

struct omnibook_feature watch_feature = {
	 .name = "watch",
	 .enabled = 0,
	 .read = watch_read,
	 .write = watch_write,
};


module_param_named(watch, watch_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(watch, "Use 0 to disable, 1 to enable register watching (DANGEROUS)");

/* End of file */
