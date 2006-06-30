/*
 * apmemu.c -- code to emulate /proc/apm
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


#define APMEMU_DRIVER_VERSION	"1.16"
#define APMEMU_APM_VERSION 	"1.2"

#define APMEMU_BATTERY_LOW	30	/* Battery low threshold */

#define APMEMU_16_BIT_SUPPORT	0x01	/* 16 bit APM BIOS */
#define APMEMU_32_BIT_SUPPORT	0x02	/* 32 bit APM BIOS */
#define APMEMU_IDLE_SLOWS_CLOCK	0x04
#define APMEMU_BIOS_DISABLED	0x08	/* APM BIOS disabled */
#define APMEMU_BIOS_DISENGAGED	0x10	/* APM BIOS disengaged */

#define APMEMU_AC_OFFLINE	0x00	/* AC offline */
#define APMEMU_AC_ONLINE	0x01	/* AC online */
#define APMEMU_AC_BACKUP	0x02	/* On backup power */
#define APMEMU_AC_UNKNOWN	0xFF	/* Unkonwn status */

#define APMEMU_BATTSTAT_HIGH	0x00	/* Remaining battery capacity is high */
#define APMEMU_BATTSTAT_LOW	0x01	/* Remaining battery capacity is low */
#define APMEMU_BATTSTAT_CRIT	0x02	/* Battery status is critical */
#define APMEMU_BATTSTAT_CHR	0x03	/* Battery is charging */
#define APMEMU_BATTSTAT_MISS	0x04	/* Battery is not present */

#define APMEMU_BATTFLAG_HIGH	0x01	/* Remaining battery capacity is high bit */
#define APMEMU_BATTFLAG_LOW	0x02	/* Remaining battery capacity is low bit */
#define APMEMU_BATTFLAG_CRIT	0x04	/* Battery status is critical bit */
#define APMEMU_BATTFLAG_CHR	0x08	/* Battery is charging bit */
#define APMEMU_BATTFLAG_MISS	0x80	/* Battery is not present bit */

#define APMEMU_BATTSTAT_UNKN	0xff	/* Status is unknown */

#define APMEMU_BATTLIFE_UNKN	-1	/* Remaining battery capacity is unknown */
