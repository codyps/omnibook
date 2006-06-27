/*
 * temperature.c -- CPU temprature monitoring
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

static int omnibook_get_cpu_temp(void)
{
	u8 temp = 0;
	int retval;
	/*
	 * XE3GF
	 * TSP10
	 * TSM30X 
	 */
	if (omnibook_ectype & (XE3GF|TSP10|TSM30X) ) {
		if ((retval = omnibook_ec_read(XE3GF_CTMP, &temp)))
			return retval;
		retval = temp;
	/*
	 * XE3GC
	 * AMILOD 
	 */
	} else if (omnibook_ectype & (XE3GC|AMILOD) ) {
		if ((retval = omnibook_ec_read(XE3GC_CTMP, &temp)))
			return retval;
		retval = temp;
	/*
	 * OB500
	 * OB510
	 * OB6000
	 * OB6100
	 * XE4500
	 * XE2
	 */
	} else if (omnibook_ectype & (OB500|OB510|OB6000|OB6100|XE4500|XE2) ) {
		if ((retval = omnibook_ec_read(OB500_CTMP, &temp)))
			return retval;
		retval = temp;
	/*
	 * OB4150 
	 */
	} else if (omnibook_ectype & (OB4150) ) {
	
		if ((retval = omnibook_ec_read(OB4150_TMP, &temp)))
			return retval;
		retval = temp;
	} else {
		printk(KERN_INFO
		       "%s: Temperature monitoring is unsupported on this machine.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}
	return retval;
}

static int omnibook_temperature_read(char *buffer)
{
	int len = 0;
	int cpu_temp;

	cpu_temp = omnibook_get_cpu_temp();
	if (cpu_temp < 0)
		return cpu_temp;

	len +=
	    sprintf(buffer + len, "CPU temperature:            %2d C\n",
		    cpu_temp);

	return len;
}

static struct omnibook_feature __declared_feature temperature_feature = {
	 .name = "temperature",
	 .enabled = 1,
	 .read = omnibook_temperature_read,
	 .ectypes = XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|OB4150|XE2|AMILOD|TSP10|TSM30X,
};

module_param_named(temperature, temperature_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(temperature, "Use 0 to disable, 1 to enable thermal status and policy support");
/* End of file */
