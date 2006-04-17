/*
 * init.h -- Interfaces functions with omnibook features
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



struct omnibook_feature;

static int __init omnibook_init(struct omnibook_feature *feature);
static int __init omnibook_probe(struct platform_device *dev);
static int __exit omnibook_remove(struct platform_device *dev);
static int omnibook_suspend(struct platform_device *dev, pm_message_t state);
static int omnibook_resume(struct platform_device *dev);

extern struct omnibook_feature ac_feature;
extern struct omnibook_feature apmemu_feature;
extern struct omnibook_feature battery_feature;
extern struct omnibook_feature blank_feature;
extern struct omnibook_feature display_feature;
extern struct omnibook_feature dock_feature;
extern struct omnibook_feature dump_feature;
extern struct omnibook_feature fan_feature;
extern struct omnibook_feature fan_policy_feature;
extern struct omnibook_feature dmi_feature;
extern struct omnibook_feature lcd_feature;
extern struct omnibook_feature onetouch_feature;
extern struct omnibook_feature temperature_feature;
extern struct omnibook_feature touchpad_feature;
extern struct omnibook_feature version_feature;
extern struct omnibook_feature watch_feature;

/* End of file */
