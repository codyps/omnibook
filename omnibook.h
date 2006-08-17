/*
 * omnibook.h -- High level data structures and functions of omnibook
 *               support code
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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/config.h>
#include <linux/version.h>

/*
 * Module informations
 */

#define OMNIBOOK_MODULE_NAME		"omnibook"
#define OMNIBOOK_MODULE_VERSION		"2.20060816"

/*
 * EC types
 */

extern int omnibook_ectype;

#define	NONE	0	/* 0  Default/unknown EC type */ 
#define	XE3GF	(1<<0)	/* 1  HP OmniBook XE3 GF, most Toshiba Satellites and more */
#define	XE3GC	(1<<1)	/* 2  HP OmniBook XE3 GC, GD, GE and compatible */
#define	OB500	(1<<2)	/* 3  HP OmniBook 500 and compatible */
#define	OB510	(1<<3)	/* 4  HP OmniBook 510 */
#define	OB6000	(1<<4)	/* 5  HP OmniBook 6000 */
#define	OB6100	(1<<5)	/* 6  HP OmniBook 6100 */
#define	XE4500	(1<<6)	/* 7  HP OmniBook xe4500 and compatible */
#define	OB4150	(1<<7)	/* 8  HP OmniBook 4150 */
#define	XE2	(1<<8)	/* 9  HP OmniBook XE2 */
#define	AMILOD	(1<<9)  /* 10 Fujitsu Amilo D */
#define	TSP10	(1<<10)	/* 11 Toshiba Satellite P10, P15, P20 and compatible */
#define	TSM30X	(1<<11)	/* 12 Toshiba Satellite M30X, M35X, M40X, M70 and compatible */
#define	TSM40	(1<<12)	/* 13 Toshiba Satellite M40 */
#define	TSA105	(1<<13)	/* 14 Toshiba Satellite A105 */
 
/*
 * This represent a feature provided by this module
 */

struct omnibook_feature {
	char *name;		/* Name */
	char *proc_entry; 	/* Specify proc entry relative to /proc (will be omnibook/name otherwise) */
	int enabled;		/* Set from module parameter */
	int (*read) (char *);	/* Procfile read function */
	int (*write) (char *);	/* Procfile write function */
	int (*init) (void);	/* Specific Initialization function */
	void (*exit) (void);	/* Specific Cleanup function */
	int (*suspend) (void);  /* PM Suspend function */
	int (*resume) (void);   /* PM Resume function */
	int ectypes;		/* Type(s) of EC we support for this feature (bitmask) */
	struct list_head list;
};

struct omnibook_battery_info {
	u8 type;		/* 1 - Li-Ion, 2 NiMH */
	u16 sn;			/* Serial number */
	u16 dv;			/* Design Voltage */
	u16 dc;			/* Design Capacity */
};
struct omnibook_battery_state {
	u16 pv;			/* Present Voltage */
	u16 rc;			/* Remaining Capacity */
	u16 lc;			/* Last Full Capacity */
	u8 gauge;		/* Gauge in % */
	u8 status;		/* 0 - unknown, 1 - charged, 2 - discharging, 3 - charging, 4 - critical) */
};

enum {
	OMNIBOOK_BATTSTAT_UNKNOWN,
	OMNIBOOK_BATTSTAT_CHARGED,
	OMNIBOOK_BATTSTAT_DISCHARGING,
	OMNIBOOK_BATTSTAT_CHARGING,
	OMNIBOOK_BATTSTAT_CRITICAL
};


extern int omnibook_lcd_blank(int blank);
extern int omnibook_get_ac(void);
extern int omnibook_get_battery_status(int num, struct omnibook_battery_state *battstat);
extern int set_omnibook_param(const char *val, struct kernel_param *kp);

#define __declared_feature __attribute__ (( __section__(".features"),  __aligned__(__alignof__ (struct omnibook_feature)))) __attribute_used__

/*
 * yet another printk wrapper
 */
#define O_INFO	KERN_INFO OMNIBOOK_MODULE_NAME ": "
#define O_WARN	KERN_WARNING OMNIBOOK_MODULE_NAME ": "
#define O_ERR	KERN_ERR OMNIBOOK_MODULE_NAME ": "

#ifdef OMNIBOOK_DEBUG
#define dprintk(fmt, args...) printk(KERN_INFO "%s: " fmt, OMNIBOOK_MODULE_NAME, ## args)
#define dprintk_simple(fmt, args...) printk(fmt, ## args)
#else
#define dprintk(fmt, args...)	do { } while(0)
#define dprintk_simple(fmt, args...) do { } while(0)
#endif

/* 
 * Configuration for standalone compilation: 
 * -Register as backlight depends on kernel config (requires 2.6.17+ interface)
 * -APM emulation is disabled by default
 */

#ifdef  OMNIBOOK_STANDALONE
#if     (defined (CONFIG_BACKLIGHT_CLASS_DEVICE_MODULE) || defined(CONFIG_BACKLIGHT_CLASS_DEVICE)) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,16))
#define CONFIG_OMNIBOOK_BACKLIGHT
#else
#undef  CONFIG_OMNIBOOK_BACKLIGHT
#endif /* BACKLIGHT_CLASS_DEVICE */
#undef	CONFIG_OMNIBOOK_APMEMU
#endif /* OMNIBOOK_STANDALONE */

/* End of file */
