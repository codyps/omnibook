/*
 * fan_policy.c -- fan policy support
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

#include <linux/ctype.h>
#include "ec.h"

/*
 * Default temperature limits.
 * Danger! You may overheat your CPU!
 * Do not change these values unless you exactly know what you do.
 */

#define OMNIBOOK_FAN_LEVELS			7
#define OMNIBOOK_FAN_MIN			25	/* Minimal value of fan off temperature */
#define OMNIBOOK_FOT_MAX			75	/* Maximal value of fan off temperature */
#define OMNIBOOK_FAN_MAX			95	/* Maximal value of fan on temperature */
#define OMNIBOOK_FOT_DEFAULT			60	/* Default value of fan off temperature */
#define OMNIBOOK_FAN1_DEFAULT			75	/* Default value of fan on temperature */
#define OMNIBOOK_FAN2_DEFAULT			85	/* Default value of fan level 2 temperature */
#define OMNIBOOK_FAN3_DEFAULT			90	/* Default value of fan level 3 temperature */
#define OMNIBOOK_FAN4_DEFAULT			95	/* Default value of fan level 4 temperature */
#define OMNIBOOK_FAN5_DEFAULT			95	/* Default value of fan level 5 temperature */
#define OMNIBOOK_FAN6_DEFAULT			95	/* Default value of fan level 6 temperature */
#define OMNIBOOK_FAN7_DEFAULT			95	/* Default value of fan level 7 temperature */

u8 omnibook_fan_policy[OMNIBOOK_FAN_LEVELS];

static int omnibook_get_fan_policy(void)
{
	int retval = 0;
	int i;
	u8 tmp;
	
	/*
	 * XE3GF
	 */
	if (omnibook_ectype & (XE3GF) ) {
		for (i = 0; i <= OMNIBOOK_FAN_LEVELS; i++) {
			if ((retval = omnibook_ec_read(XE3GF_FOT + i, &tmp)))
				return retval;
			omnibook_fan_policy[i] = tmp;
		}
	} else {
		printk(KERN_INFO
		       "%s: Fan policy is unsupported on this machine.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}

	return retval;
}

static int omnibook_set_fan_policy(void)
{
	int retval;
	int i;

	/*
	 * XE3GF
	 */
	if (omnibook_ectype & (XE3GF) ) {
		if (omnibook_fan_policy[0] > OMNIBOOK_FOT_MAX)
			return -EINVAL;
		for (i = 0; i < OMNIBOOK_FAN_LEVELS; i++) {
			if ((omnibook_fan_policy[i] >
			     omnibook_fan_policy[i + 1])
			    || (omnibook_fan_policy[i] < OMNIBOOK_FAN_MIN)
			    || (omnibook_fan_policy[i] > OMNIBOOK_FAN_MAX))
				return -EINVAL;
			if (omnibook_fan_policy[i + 1] > OMNIBOOK_FAN_MAX)
				return -EINVAL;
		}
		for (i = 0; i <= OMNIBOOK_FAN_LEVELS; i++) {
			if ((retval =
			     omnibook_ec_write(XE3GF_FOT + i, omnibook_fan_policy[i])))
				return retval;
		}
	} else {
		printk(KERN_INFO
		       "%s: Fan policy is unsupported on this machine.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}

	return retval;
}

static int omnibook_set_fan_policy_defaults(void)
{
	int retval;
	int i;
	u8 fan_defaults[] = {
		OMNIBOOK_FOT_DEFAULT,
		OMNIBOOK_FAN1_DEFAULT,
		OMNIBOOK_FAN2_DEFAULT,
		OMNIBOOK_FAN3_DEFAULT,
		OMNIBOOK_FAN4_DEFAULT,
		OMNIBOOK_FAN5_DEFAULT,
		OMNIBOOK_FAN6_DEFAULT,
		OMNIBOOK_FAN7_DEFAULT,
	};

	/*
	 * XE3GF
	 */
	if (omnibook_ectype & (XE3GF) ) {
		for (i = 0; i <= OMNIBOOK_FAN_LEVELS; i++) {
			if ((retval = omnibook_ec_write(XE3GF_FOT + i, fan_defaults[i])))
				return retval;
		}
	} else {
		printk(KERN_INFO
		       "%s: Fan policy is unsupported on this machine.\n",
		       OMNIBOOK_MODULE_NAME);
		retval = -ENODEV;
	}

	return retval;
}

static int omnibook_fan_policy_read(char *buffer)
{
	int retval;
	int len = 0;
	u8 i;

	if ((retval = omnibook_get_fan_policy()))
		return retval;

	len +=
	    sprintf(buffer + len, "Fan off temperature:        %2d C\n",
		    omnibook_fan_policy[0]);
	len +=
	    sprintf(buffer + len, "Fan on temperature:         %2d C\n",
		    omnibook_fan_policy[1]);
	for (i = 2; i <= OMNIBOOK_FAN_LEVELS; i++) {
		len +=
		    sprintf(buffer + len,
			    "Fan level %1d temperature:    %2d C\n", i,
			    omnibook_fan_policy[i]);
	}
	len +=
	    sprintf(buffer + len, "Minimal temperature to set: %2d C\n",
		    OMNIBOOK_FAN_MIN);
	len +=
	    sprintf(buffer + len, "Maximal temperature to set: %2d C\n",
		    OMNIBOOK_FAN_MAX);

	return len;
}

static int omnibook_fan_policy_write(char *buffer)
{
	int n = 0;
	char *b;
	char *endp;
	int retval;
	int temp;

	if ((retval = omnibook_get_fan_policy()))
		return retval;

	/* 
	 * Could also be done much simpler using sscanf(,"%u %u ... 
	 * but this would hardcode OMNIBOOK_FAN_LEVELS.
	 * The parsed format is "%u " repeated OMNIBOOK_FAN_LEVELS+1 times
	 */

	b = buffer;
	do {

		pr_debug("n=[%i] b=[%s]\n", n, b);

		if (n > OMNIBOOK_FAN_LEVELS)
			return -EINVAL;
		if (!isspace(*b)) {
			temp = simple_strtoul(b, &endp, 10);
			if (endp != b) {	/* there was a match */
				omnibook_fan_policy[n++] = temp;
				b = endp;
			} else
				return -EINVAL;
		} else
			b++;
	} while ((*b != '\0') && (*b != '\n'));

	/* A zero value set the defaults */
	if ((omnibook_fan_policy[0] == 0) && (n == 1)) {
		if ((retval = omnibook_set_fan_policy_defaults()))
			return retval;
	} else if ((retval = omnibook_set_fan_policy()))
		return retval;
	return 0;
}

static struct omnibook_feature __declared_feature fan_policy_feature = {
	 .name = "fan_policy",
	 .enabled = 1,
	 .read = omnibook_fan_policy_read,
	 .write = omnibook_fan_policy_write,
	 .ectypes = XE3GF,
};

module_param_named(fan_policy, fan_policy_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(fan_policy, "Use 0 to disable, 1 to enable fan control policy support");
/* End of file */
