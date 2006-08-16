/*
 * init.c -- module initialization code
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

#include <linux/proc_fs.h>
#include <linux/dmi.h>
#include <linux/version.h>
#include <asm/uaccess.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15))
#include <linux/platform_device.h>
#else
#include <linux/device.h>
#endif

#include "ec.h"
#include "laptop.h"
#include "compat.h"

static struct proc_dir_entry *omnibook_proc_root = NULL;

int omnibook_ectype = NONE;
static char* laptop_model __initdata;

static int omnibook_userset = 0;

/*
 * The platform_driver interface was added in linux 2.6.15
 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15))

static struct platform_device *omnibook_device;

static struct platform_driver omnibook_driver = {
	.probe = omnibook_probe,
	.remove = omnibook_remove,
#ifdef CONFIG_PM
	.suspend = omnibook_suspend,
	.resume = omnibook_resume,
#endif
	.driver = {
		   .name = OMNIBOOK_MODULE_NAME,
		   .owner  = THIS_MODULE,
		   },
};

#else /* 2.6.15 */

static struct device_driver omnibook_driver = {
	.name   = OMNIBOOK_MODULE_NAME,
	.bus    = &platform_bus_type,
	.probe = compat_omnibook_probe,
	.remove = compat_omnibook_remove,
#ifdef CONFIG_PM
	.suspend = compat_omnibook_suspend,
	.resume = compat_omnibook_resume,
#endif
};
 
static struct platform_device omnibook_device = {
	.name   = OMNIBOOK_MODULE_NAME,
};

#endif /* 2.6.15 */


/* Linked list of all enabled features */
static struct omnibook_feature *omnibook_available_feature;

/* Delimiters of the .features section wich holds all the omnibook_feature structs */
extern struct omnibook_feature _features_start[];
extern struct omnibook_feature _features_end[];

static int __init dmi_matched(struct dmi_system_id *dmi)
{
	omnibook_ectype = (int) dmi->driver_data;
	if (dmi->ident)
		laptop_model = (char*) dmi->ident;
	else
		laptop_model = dmi_get_system_info(DMI_PRODUCT_VERSION);
	return 0;
}

/* 
 * Callback function for procfs file reading: the name of the file read was stored in *data 
 */
static int procfile_read_dispatch(char *page, char **start, off_t off,
				  int count, int *eof, void *data)
{
	struct omnibook_feature *feature = (struct omnibook_feature *)data;
	int len;

	if (!feature || !feature->read)
		return -EINVAL;

	len = feature->read(page);
	if (len < 0)
		return len;

	if (len <= off + count)
		*eof = 1;
	*start = page + off;
	len -= off;
	if (len > count)
		len = count;
	if (len < 0)
		len = 0;

	return len;
}

/* 
 * Callback function for procfs file writing: the name of the file written was stored in *data 
 */
static int procfile_write_dispatch(struct file *file,
				   const char __user * userbuf,
				   unsigned long count, void *data)
{
	struct omnibook_feature *feature = (struct omnibook_feature *)data;
	char *kernbuf;
	int retval;

	if (!feature || !feature->write)
		return -EINVAL;

	kernbuf = kmalloc(count + 1, GFP_KERNEL);
	if (!kernbuf)
		return -ENOMEM;

	if (copy_from_user(kernbuf, userbuf, count)) {
		kfree(kernbuf);
		return -EFAULT;
	}

	/* Make sure the string is \0 terminated */
	kernbuf[count] = '\0';

	retval = feature->write(kernbuf);
	if (retval == 0)
		retval = count;

	kfree(kernbuf);

	return retval;
}

/* 
 * Initialise a feature and add it to the linked list of active features
 */
static int __init omnibook_init(struct omnibook_feature *feature)
{
	int retval;
	mode_t pmode;
	struct proc_dir_entry *proc_entry;

	if (!feature)
		return -EINVAL;

	if (feature->init) {
		retval = feature->init();
		if (retval)
			return -ENODEV;
	}
	if (feature->name && feature->read) {
		pmode = S_IFREG | S_IRUGO;
		if (feature->write) {
			pmode |= S_IWUSR;
			if (omnibook_userset)
				pmode |= S_IWUGO;
		}
		/*
		 * Special case for apmemu
		 */
		if (feature->proc_entry) {
			proc_entry = create_proc_entry(feature->proc_entry, pmode,
						      NULL);
		} else {
		proc_entry = create_proc_entry(feature->name, pmode,
					       omnibook_proc_root);
		}
		if (!proc_entry) {
			printk(O_ERR
			       "Unable to create proc entry %s\n",feature->name);
			if (feature->exit)
				feature->exit();
			return -ENOENT;
		}
		proc_entry->data = feature;
		proc_entry->read_proc = &procfile_read_dispatch;
		if (feature->write)
			proc_entry->write_proc = &procfile_write_dispatch;
		proc_entry->owner = THIS_MODULE;
	}
	list_add_tail(&feature->list, &omnibook_available_feature->list);
	return 0;
}


/* 
 * Callback function for driver registering :
 * Initialize the linked list of enabled features and call omnibook_init to populate it
 */
static int __init omnibook_probe(struct platform_device *dev)
{
	int i;
	struct list_head *p;
	struct omnibook_feature *feature;

	
	omnibook_available_feature =
	    kzalloc(sizeof(struct omnibook_feature), GFP_KERNEL);
	if (!omnibook_available_feature)
		return -ENOMEM;
	INIT_LIST_HEAD(&omnibook_available_feature->list);
	
	for (i=0; i < _features_end - _features_start; i++ ) {
		
		feature = &_features_start[i];

		if (!feature->enabled)	
			continue;
		
		if ((omnibook_ectype & feature->ectypes) || (!feature->ectypes))
			omnibook_init(feature);
	}
	
	printk(O_INFO "Enabled features:");
	list_for_each(p, &omnibook_available_feature->list) {
		feature = list_entry(p, struct omnibook_feature, list);
		if (feature->name)
			printk(" %s", feature->name);
	}
	printk(".\n");
	
	return 0;
}

/* 
 * Callback function for driver removal
 */
static int __exit omnibook_remove(struct platform_device *dev)
{
	struct list_head *p, *n;
	struct omnibook_feature *feature;

	list_for_each_safe(p, n, &omnibook_available_feature->list) {
		feature = list_entry(p, struct omnibook_feature, list);
		list_del(p);
		if (feature->exit)
			feature->exit();
		if (feature->proc_entry)
			remove_proc_entry(feature->proc_entry, NULL);
		else if (feature->name)
			remove_proc_entry(feature->name, omnibook_proc_root);
	}
	kfree(omnibook_available_feature);
	
	return 0;
}

/* 
 * Callback function for system suspend 
 */
static int omnibook_suspend(struct platform_device *dev, pm_message_t state)
{
	int retval;
	struct list_head *p;
	struct omnibook_feature *feature;

	list_for_each(p, &omnibook_available_feature->list) {
		feature = list_entry(p, struct omnibook_feature, list);
		if (feature->suspend) {
			retval = feature->suspend();
			if (!retval)
				printk(O_ERR
				        "Unable to suspend the %s feature",
				        feature->name);
		}
	}
	return 0;
}

/* 
 * Callback function for system resume
 */
static int omnibook_resume(struct platform_device *dev)
{
	int retval;
	struct list_head *p;
	struct omnibook_feature *feature;

	list_for_each(p, &omnibook_available_feature->list) {
		feature = list_entry(p, struct omnibook_feature, list);
		if (feature->resume) {
			retval = feature->resume();
			if (!retval)
				printk(O_ERR
				       "Unable to resume the %s feature",
				       feature->name);
		}
	}
	return 0;
}

/*
 * Maintain compatibility with the old ectype numbers:
 * ex: The user set/get ectype=12 for TSM30X=2^(12-1)
 */
static int __init set_ectype_param(const char *val, struct kernel_param *kp)
{
	char *endp;
	int value;

	if (!val) return -EINVAL;
	
	value = simple_strtol(val, &endp, 10);
	if (endp == val) /* No match */
		return -EINVAL;
	omnibook_ectype = 1 << ( value - 1);
	return 0;
}

static int get_ectype_param(char *buffer, struct kernel_param *kp)
{
	return sprintf(buffer,"%i", ffs(omnibook_ectype));
}

static int __init omnibook_module_init(void)
{
	int retval;

	printk(O_INFO "Driver version %s.\n", OMNIBOOK_MODULE_VERSION);
	    
	if (omnibook_ectype != NONE)
		printk(O_WARN
		       "Forced load with EC firmware type %i.\n", ffs(omnibook_ectype));
	else if ( dmi_check_system(omnibook_ids) ) 
		printk(O_INFO "%s detected.\n", laptop_model);
	else
		printk(O_INFO "Unknown model.\n");

	omnibook_proc_root = proc_mkdir(OMNIBOOK_MODULE_NAME, NULL);
	if (!omnibook_proc_root) {
		printk(O_ERR "Unable to create /proc/%s.\n", OMNIBOOK_MODULE_NAME);
		return -ENOENT;
	}

/*
 * The platform_driver interface was added in linux 2.6.15
 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15))

	retval = platform_driver_register(&omnibook_driver);
	if (retval < 0) return retval;

	omnibook_device = platform_device_alloc(OMNIBOOK_MODULE_NAME, -1);
	if (!omnibook_device) {
		platform_driver_unregister(&omnibook_driver);
		return -ENOMEM;
	}
	
	retval = platform_device_add(omnibook_device);
	if (retval) {
		platform_device_put(omnibook_device);
		platform_driver_unregister(&omnibook_driver);
		return retval;
	}

#else /* 2.6.15 */

	retval = driver_register(&omnibook_driver);
	if (retval < 0) return retval;
        
        retval = platform_device_register(&omnibook_device);
        
        if (retval) {
        	driver_unregister(&omnibook_driver);	
		return retval;
	}

#endif
	return 0;
}

static void __exit omnibook_module_cleanup(void)
{

/*
 * The platform_driver interface was added in linux 2.6.15
 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15))
	platform_device_unregister(omnibook_device);
	platform_driver_unregister(&omnibook_driver);
#else
	platform_device_unregister(&omnibook_device);
	driver_unregister(&omnibook_driver);
#endif

	if (omnibook_proc_root)
		remove_proc_entry("omnibook", NULL);
	printk(O_INFO "Module is unloaded.\n");
}

module_init(omnibook_module_init);
module_exit(omnibook_module_cleanup);

MODULE_AUTHOR("Soós Péter, Mathieu Bérard");
MODULE_DESCRIPTION("Kernel interface for HP OmniBook, HP Pavilion, Toshiba Satellite, Acer Aspire and Compal ACL00 laptops");
MODULE_LICENSE("GPL");
module_param_call(ectype, set_ectype_param, get_ectype_param, NULL, S_IRUGO);
module_param_named(userset, omnibook_userset, int, S_IRUGO);
MODULE_PARM_DESC(ectype, "Type of embedded controller firmware");
MODULE_PARM_DESC(userset, "Use 0 to disable, 1 to enable users to set parameters");


/* End of file */
