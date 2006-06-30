/*
 * apmemu.c -- /proc/apm emulation
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


#ifdef CONFIG_APM
#include <linux/apm_bios.h>
#endif

#include <asm/system.h>
#include <asm/io.h>

#include "apmemu.h"

/* Arguments, with symbols from linux/apm_bios.h.  Information is
   from the Get Power Status (0x0a) call unless otherwise noted.
   0) Linux driver version (this will change if format changes)
   1) APM BIOS Version.  Usually 1.0, 1.1 or 1.2.
   2) APM flags from APM Installation Check (0x00):
      bit 0: APM_16_BIT_SUPPORT
      bit 1: APM_32_BIT_SUPPORT
      bit 2: APM_IDLE_SLOWS_CLOCK
      bit 3: APM_BIOS_DISABLED
      bit 4: APM_BIOS_DISENGAGED
   3) AC line status
      0x00: Off-line
      0x01: On-line
      0x02: On backup power (BIOS >= 1.1 only)
      0xff: Unknown
   4) Battery status
      0x00: High
      0x01: Low
      0x02: Critical
      0x03: Charging
      0x04: Selected battery not present (BIOS >= 1.2 only)
      0xff: Unknown
   5) Battery flag
      bit 0: High
      bit 1: Low
      bit 2: Critical
      bit 3: Charging
      bit 7: No system battery
      0xff: Unknown
   6) Remaining battery life (percentage of charge):
      0-100: valid
      -1: Unknown
   7) Remaining battery life (time units):
      Number of remaining minutes or seconds
      -1: Unknown
   8) min = minutes; sec = seconds */

static int omnibook_apmemu_read(char *buffer)
{
	int retval;
	int len = 0;
	int ac;
	
	struct omnibook_battery_state battstat;
	
	struct apm_features {
		char *drvver;
		char *apmver;
		u8 apmflags;
		u8 ac;
		u8 battstat;
		u8 battflags;
		u8 gauge;
		int time;
		char *units;
	};

	struct apm_features apm = {
		APMEMU_DRIVER_VERSION,
		APMEMU_APM_VERSION,
		APMEMU_32_BIT_SUPPORT | APMEMU_BIOS_DISABLED,
		APMEMU_AC_UNKNOWN,
		APMEMU_BATTSTAT_UNKN,
		0x00,
		APMEMU_BATTLIFE_UNKN,
		APMEMU_BATTLIFE_UNKN,
		"?"
	};

	ac = omnibook_get_ac();
	apm.ac = (ac) ? APMEMU_AC_ONLINE : APMEMU_AC_OFFLINE;
	/* Asking for Battery 0 as APM does */
	retval = omnibook_get_battery_status(0, &battstat);
	if (retval == 0)
		apm.gauge = battstat.gauge;
	if (apm.gauge >= APMEMU_BATTERY_LOW) {
		apm.battflags = apm.battflags | APMEMU_BATTFLAG_HIGH;
		apm.battstat = APMEMU_BATTSTAT_HIGH;
	} else {
		apm.battflags = apm.battflags | APMEMU_BATTFLAG_LOW;
		apm.battstat = APMEMU_BATTSTAT_LOW;
	}
	if (battstat.status == OMNIBOOK_BATTSTAT_CHARGING) {
		apm.battflags = apm.battflags | APMEMU_BATTFLAG_CHR;
		apm.battstat = APMEMU_BATTSTAT_CHR;
	}
	if (battstat.status == OMNIBOOK_BATTSTAT_CRITICAL) {
		apm.battflags = apm.battflags | APMEMU_BATTFLAG_CRIT;
		apm.battstat = APMEMU_BATTSTAT_CRIT;
	}

	len += sprintf(buffer + len, "%s %s 0x%02x 0x%02x 0x%02x 0x%02x %d%% %d %s\n",
			apm.drvver,
			apm.apmver,
			apm.apmflags,
			apm.ac,
			apm.battstat,
			apm.battflags,
			apm.gauge,
			apm.time,
			apm.units
	);

	return len;
}

static int omnibook_apmemu_init(void)
{
#ifdef CONFIG_APM
	if (!apm_info.disabled) {
		printk(KERN_INFO "%s: Real APM support is present, emulation is not necessary.\n", OMNIBOOK_MODULE_NAME);
		return -ENODEV;
	}
#endif
	return 0;
}

static struct omnibook_feature __declared_feature apmemu_feature = {
	 .name = "apmemu",
	 .proc_entry = "apm", /* create /proc/apm */
#ifdef CONFIG_OMNIBOOK_APMEMU
         .enabled = 1,
#else
	 .enabled = 0,
#endif
	 .read = omnibook_apmemu_read,
	 .init = omnibook_apmemu_init,
	 .ectypes = XE3GF|XE3GC|TSP10,
};


module_param_named(apmemu, apmemu_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(apmemu, "Use 0 to disable, 1 to enable /proc/apm emulation");

/* End of file */
