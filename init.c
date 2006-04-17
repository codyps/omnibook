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
 */

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include <linux/proc_fs.h>
#include <linux/dmi.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>

#include "init.h"
#include "ec.h"


static struct proc_dir_entry *omnibook_proc_root = NULL;

int omnibook_ectype = NONE;
static int omnibook_userset = 0;

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

/* Linked list of all enabled features */
static struct omnibook_feature *omnibook_available_feature;

/* 
 * Array of all the features provided by this module 
 */
static struct omnibook_feature *omnibook_features[] __initdata = {
	&ac_feature,
	&apmemu_feature,
	&battery_feature,
	&blank_feature,
	&display_feature,
	&dock_feature,
	&dump_feature,
	&fan_feature,
	&fan_policy_feature,
	&dmi_feature,
	&lcd_feature,
	&onetouch_feature,
	&temperature_feature,
	&touchpad_feature,
	&version_feature,
	&watch_feature
};

#define HP_SIGNATURE	"Hewlett-Packard"

struct omnibook_models_t {
	/* DMI field matchers (table inputs) */
	char *sys_vendor;
	char *product_name;
	char *product_version;
	char *board_name;
	/* Table outputs */
	char *syslog_name;	/* Name which will appear in the syslog */
	int ectype;	/* Type of the embedded controller firmware, see omnibook.h and README */
};

static struct omnibook_models_t omnibook_models[] __initdata = {
  /* sys_vendor product_name                 product_version                   board_name syslog_name                  ectype */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XE3 GF*",            NULL,    NULL,                          XE3GF },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XT1000*",            NULL,    NULL,                          XE3GF },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XE2 DC*",            NULL,    NULL,                          XE2 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XE3 GC*",            NULL,    NULL,                          XE3GC },
  /* HP Pavilion N5430 */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XE3 GD*",            NULL,    NULL,                          XE3GC },
  /* HP Pavilion N5415 */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XE3 GE*",            NULL,    NULL,                          XE3GC },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 500 FA*",            NULL,    NULL,                          OB500 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 510 FB*",            NULL,    NULL,                          OB510 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 4150*",              NULL,    NULL,                          OB4150 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 900 B*",             NULL,    NULL,                          OB4150 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 6000 EA*",           NULL,    NULL,                          OB6000 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 6100 EB*",           NULL,    NULL,                          OB6100 },
  /* HP OmniBook xe4100 */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook xe4000*",            NULL,    NULL,                          XE4500 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook xe4400*",            NULL,    NULL,                          XE4500 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook xe4500*",            NULL,    NULL,                          XE4500 },
  /* HP OmniBook vt6200 and xt6200 */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 6200 EG*",           NULL,    NULL,                          XE4500 },
  /* There are no model specific strings of some HP OmniBook XT1500 */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook*",                   NULL,    NULL,                          XE3GF },
  /* HP Pavilion ze4125 */
  { NULL,       "HP NoteBook PC*",           "HP NoteBook ze4000*",            NULL,    NULL,                          XE4500 },
  /* There are no model specific strings of some HP Pavilion xt155 and some HP Pavilion ze4100 */
  { NULL,       "HP NoteBook PC*",           "HP NoteBook PC*",                NULL,    NULL,                          XE4500 },
  /* There are no model specific strings of some HP nx9000 */
  { NULL,       "HP Notebook PC*",           "HP Notebook PC*",                NULL,    NULL,                          XE4500 },
  /* HP Pavilion ZU1155 and ZU1175 */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion ZU1000 FA*",         NULL,    NULL,                          OB500 },
  /* HP Pavilion N5290 */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion Notebook XE3 GC*",   NULL,    NULL,                          XE3GC },
  /* HP Pavilion N5441 */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion Notebook Model GD*", NULL,    NULL,                          XE3GC },
  /* HP Pavilion XH545 */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion Notebook Model GE*", NULL,    NULL,                          XE3GC },
  /* HP Pavilion ZT1141 */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion Notebook ZT1000*",   NULL,    NULL,                          XE3GF },
  /* There are no model specific strings of some HP Pavilion ZT1175 and ZT1195 notebooks */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion Notebook*",          NULL,    NULL,                          XE3GF },
  { NULL,       "Pavilion ze4200*",          NULL,                             NULL,    "HP Pavilion ze4200 series",   XE4500 },
  { NULL,       "Pavilion ze4300*",          NULL,                             NULL,    "HP Pavilion ze4300 series",   XE4500 },
  { NULL,       "Pavilion ze8500*",          NULL,                             NULL,    "HP Pavilion ze8500 series",   XE4500 },
  /* Compaq nx9000 */
  { NULL,       "HP nx9000*",                NULL,                             NULL,    "HP Compaq nx9000",            XE4500 },
  /* Compaq nx9005 */
  { NULL,       "HP nx9005*",                NULL,                             NULL,    "HP Compaq nx9005",            XE4500 },
  /* Compaq nx9010 */
  { NULL,       "HP nx9010*",                NULL,                             NULL,    "HP Compaq nx9010",            XE4500 },
  { "TOSHIBA",  "S1000*",                    NULL,                             NULL,    "Toshiba Satellite 1000",      XE3GF },
  { "TOSHIBA",  "S1005*",                    NULL,                             NULL,    "Toshiba Satellite 1005",      XE3GF },
  { "TOSHIBA",  "S1110*",                    NULL,                             NULL,    "Toshiba Satellite 1110",      XE3GF },
  { "TOSHIBA",  "S1115*",                    NULL,                             NULL,    "Toshiba Satellite 1115",      XE3GF },
  { "TOSHIBA",  "S1900*",                    NULL,                             NULL,    "Toshiba Satellite 1900",      XE3GF },
  { "TOSHIBA",  "S1905*",                    NULL,                             NULL,    "Toshiba Satellite 1905",      XE3GF },
  { "TOSHIBA",  "S1950*",                    NULL,                             NULL,    "Toshiba Satellite 1950",      XE3GF },
  { "TOSHIBA",  "S1955*",                    NULL,                             NULL,    "Toshiba Satellite 1955",      XE3GF },
  { "TOSHIBA",  "S2430*",                    NULL,                             NULL,    "Toshiba Satellite 2430",      XE3GF },
  { "TOSHIBA",  "S2435*",                    NULL,                             NULL,    "Toshiba Satellite 2435",      XE3GF },
  { "TOSHIBA",  "S3000*",                    NULL,                             NULL,    "Toshiba Satellite 3000",      XE3GF },
  { "TOSHIBA",  "S3005*",                    NULL,                             NULL,    "Toshiba Satellite 3005",      XE3GF },
  { "TOSHIBA",  "Satellite 1000*",           NULL,                             NULL,    "Toshiba Satellite 1000",      XE3GF },
  { "TOSHIBA",  "Satellite 1005*",           NULL,                             NULL,    "Toshiba Satellite 1005",      XE3GF },
  { "TOSHIBA",  "Satellite 1110*",           NULL,                             NULL,    "Toshiba Satellite 1110",      XE3GF },
  { "TOSHIBA",  "Satellite 1115*",           NULL,                             NULL,    "Toshiba Satellite 1115",      XE3GF },
  { "TOSHIBA",  "Toshiba 1115*",             NULL,                             NULL,    "Toshiba Satellite 1115",      XE3GF },
  { "TOSHIBA",  "Satellite 1900*",           NULL,                             NULL,    "Toshiba Satellite 1900",      XE3GF },
  { "TOSHIBA",  "Satellite 1905*",           NULL,                             NULL,    "Toshiba Satellite 1905",      XE3GF },
  { "TOSHIBA",  "Satellite 1950*",           NULL,                             NULL,    "Toshiba Satellite 1950",      XE3GF },
  { "TOSHIBA",  "Satellite 1955*",           NULL,                             NULL,    "Toshiba Satellite 1955",      XE3GF },
  { "TOSHIBA",  "Satellite 2430*",           NULL,                             NULL,    "Toshiba Satellite 2430",      XE3GF },
  { "TOSHIBA",  "Satellite 2435*",           NULL,                             NULL,    "Toshiba Satellite 2435",      XE3GF },
  { "TOSHIBA",  "Satellite 3000*",           NULL,                             NULL,    "Toshiba Satellite 3000",      XE3GF },
  { "TOSHIBA",  "Satellite 3005*",           NULL,                             NULL,    "Toshiba Satellite 3005",      XE3GF },
  { "TOSHIBA",  "Satellite P10*",            NULL,                             NULL,    "Toshiba Satellite P10",       TSP10 },
  { "TOSHIBA",  "Satellite P15*",            NULL,                             NULL,    "Toshiba Satellite P15",       TSP10 },
  { "TOSHIBA",  "Satellite P20*",            NULL,                             NULL,    "Toshiba Satellite P20",       TSP10 },
  { "TOSHIBA",  "Satellite M30X*",           NULL,                             NULL,    "Toshiba Satellite M30X",      TSM30X },
  { "TOSHIBA",  "Satellite M35X*",           NULL,                             NULL,    "Toshiba Satellite M35X",      TSM30X },
  { "TOSHIBA",  "Satellite M70*",            NULL,                             NULL,    "Toshiba Satellite M70",       TSM30X },
  { "TOSHIBA",  "Satellite M40*",            NULL,                             NULL,    "Toshiba Satellite M40",       TSM40 },
  { "COMPAL",   NULL,                        NULL,                             "ACL00", "Compal ACL00",                XE3GF },
  { "COMPAL",   NULL,                        NULL,                             "ACL10", "Compal ACL10",                XE3GF },
  { "Acer",     "Aspire 1400 series*",       NULL,                             NULL,    "Acer Aspire 1400 series",     XE3GF },
  { "Acer",     "Aspire 1350*",              NULL,                             NULL,    "Acer Aspire 1350",            XE4500 },
  { "FUJITSU SIEMENS", "Amilo D-Series*",    NULL,                             NULL,    "Fujitsu-Siemens Amilo D series", AMILOD },
  /* This sentinel at the end catches all unsupported models */
  { NULL, NULL, NULL, NULL, NULL, NONE }
};

struct omnibook_tc_t {
	char *tc;
	int ectype;
};

/* HP technology codes */
static struct omnibook_tc_t omnibook_tc[] __initdata = {
	/* technology code      ectype */
	{"CI.", OB4150},
	{"CL.", OB4150},
	{"DC.", XE2},
	{"EA.", OB6000},
	{"EB.", OB6100},
	{"EG.", XE4500},
	{"FA.", OB500},
	{"FB.", OB510},
	{"GC.", XE3GC},
	{"GD.", XE3GC},
	{"GE.", XE3GC},
	{"GF.", XE3GF},
	{"IB.", XE3GF},
	{"IC.", XE3GF},
	{"ID.", XE3GF},
	{"KA.", XE4500},
	{"KB.", XE4500},
	{"KC.", XE4500},
	{"KD.", XE4500},
	{"KE.", XE4500},
	{"KE_KG.", XE4500},
	{"KF_KH.", XE4500},
	{NULL, NONE}
};

/*
 * Compare the saved DMI info at "index" with a string.
 * A '*' at the end of the string will match anything.
 * Returns 0 for a match.
 * 
 * This preserves the semantics of the old omnibook_features[]
 * table.  I don't know if its generally useful or not.
 */
static int __init cmp_with_glob(int index, char *str)
{
	int retval = 0;
	char *glob;
	unsigned int len;

	if (str) {
		glob = strchr(str, '*');
		len = glob ? glob - str : strlen(str);
		retval = strncmp(dmi_get_system_info(index), str, len);
	}

	return retval;
}

static int __init omnibook_ident(void)
{
	struct omnibook_models_t *mp;

	for (mp = omnibook_models; mp->ectype != NONE; ++mp) {
		/* Check all fields for a match */
		if (cmp_with_glob(DMI_SYS_VENDOR, mp->sys_vendor))
			continue;
		if (cmp_with_glob(DMI_PRODUCT_NAME, mp->product_name))
			continue;
		if (cmp_with_glob(DMI_PRODUCT_VERSION, mp->product_version))
			continue;
		if (cmp_with_glob(DMI_BOARD_NAME, mp->board_name))
			continue;

		/* All required fields matched */
		break;
	}

	return (mp - omnibook_models);
}

static int __init omnibook_get_tc(void)
{
	struct omnibook_tc_t *tc;

	for (tc = omnibook_tc; tc->ectype != NONE; ++tc) {
		/*
		 * Technology code appears in the first two chracters of BIOS version string
		 * ended by a dot, but it prefixed a space character on some models and BIOS
		 * versions.
		 * New HP/Compaq models use more characters (eg. KF_KH.).
		 */
		if (strstr(dmi_get_system_info(DMI_BIOS_VERSION), tc->tc))
			break;
	}

	return (tc - omnibook_tc);
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
#ifdef OMNIBOOK_DEBUG
	printk("Input string length is %u and is: [%s]\n", (unsigned int)count,
	       kernbuf);
#endif
	retval = feature->write(kernbuf);
	if (retval == 0)
		retval = count;

	kfree(kernbuf);

	return retval;
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

	for (i = 0; i < ARRAY_SIZE(omnibook_features); i++) {
		if (!omnibook_features[i]->enabled)
			continue;
		if ((omnibook_ectype & omnibook_features[i]->ectypes) || (!omnibook_features[i]->ectypes))
			omnibook_init(omnibook_features[i]);
	}
	
	printk(KERN_INFO "%s: Enabled features:",OMNIBOOK_MODULE_NAME);
	list_for_each(p, &omnibook_available_feature->list) {
		feature = list_entry(p, struct omnibook_feature, list);
		if (feature->name)
			printk(" %s", feature->name);
	}
	printk(".\n");
	
	return 0;
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
	if (feature->name) {
		if (feature->read) {
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
				proc_entry =
				    create_proc_entry(feature->proc_entry, pmode,
						     NULL);
			} else {
			proc_entry =
			    create_proc_entry(feature->name, pmode,
					      omnibook_proc_root);
			}
			if (!proc_entry) {
				printk(KERN_ERR
				       "%s: unable to create proc entry %s\n",
				       OMNIBOOK_MODULE_NAME, feature->name);
				return -ENOENT;
			}
			proc_entry->data = feature;
			proc_entry->read_proc = &procfile_read_dispatch;
			if (feature->write)
				proc_entry->write_proc =
				    &procfile_write_dispatch;
		}
	}
	list_add_tail(&feature->list, &omnibook_available_feature->list);
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
				printk(KERN_ERR
				       "%s: unable to suspend the %s feature",
				       OMNIBOOK_MODULE_NAME, feature->name);
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
				printk(KERN_ERR
				       "%s: unable to resume the %s feature",
				       OMNIBOOK_MODULE_NAME, feature->name);
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
	int model = 0;
	int tc = 0;
	char *syslog_name;
	char *glob;
	int retval;

	printk(KERN_INFO "%s: Driver version %s.\n", OMNIBOOK_MODULE_NAME,
	       OMNIBOOK_MODULE_VERSION);
	    
	if (omnibook_ectype != NONE)
		printk(KERN_WARNING
		       "%s: Forced load with EC firmware type %i.\n",
		       OMNIBOOK_MODULE_NAME, ffs(omnibook_ectype));

	else {
		model = omnibook_ident();
		if (omnibook_models[model].ectype != NONE) {
			omnibook_ectype = omnibook_models[model].ectype;
			syslog_name = omnibook_models[model].syslog_name;
			if (!syslog_name) {
				syslog_name =
				    omnibook_models[model].product_version;
				glob = strchr(syslog_name, '*');
				if (glob)
					*glob = '\0';
			}
			printk(KERN_INFO "%s: %s detected.\n",
			       OMNIBOOK_MODULE_NAME, syslog_name);
		} else {
			/* Without explicit informations try chechking for technology code of HP laptops */
			tc = omnibook_get_tc();
			if ((strncmp
			     (dmi_get_system_info(DMI_SYS_VENDOR), HP_SIGNATURE,
			      strlen(HP_SIGNATURE)) == 0)
			    && (omnibook_tc[tc].ectype != NONE)) {
				omnibook_ectype = omnibook_tc[tc].ectype;
				printk(KERN_INFO
				       "%s: HP tecnology code %s detected.\n",
				       OMNIBOOK_MODULE_NAME,
				       omnibook_tc[tc].tc);
			} else {
				printk(KERN_INFO
				       "%s: Unknown model detected.\n",
				       OMNIBOOK_MODULE_NAME);
			}
		}
	}

	omnibook_proc_root = proc_mkdir(OMNIBOOK_MODULE_NAME, NULL);
	if (!omnibook_proc_root) {
		printk(KERN_ERR "%s: Unable to create /proc/%s.\n",
		       OMNIBOOK_MODULE_NAME, OMNIBOOK_MODULE_NAME);
		return -ENOENT;
	}

	retval = platform_driver_register(&omnibook_driver);
	if (retval < 0)
		return retval;

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

	return 0;
}

static void __exit omnibook_module_cleanup(void)
{
	platform_device_unregister(omnibook_device);
	platform_driver_unregister(&omnibook_driver);

	if (omnibook_proc_root)
		remove_proc_entry("omnibook", NULL);
	printk(KERN_INFO "%s: module is unloaded.\n", OMNIBOOK_MODULE_NAME);
}

module_init(omnibook_module_init);
module_exit(omnibook_module_cleanup);

MODULE_AUTHOR("Soós Péter <sp@osb.hu>");
MODULE_DESCRIPTION("Kernel interface for HP OmniBook, HP Pavilion, Toshiba Satellite, Acer Aspire and Compal ACL00 laptops");
MODULE_LICENSE("GPL");
module_param_call(ectype, set_ectype_param, get_ectype_param, NULL, S_IRUGO);
module_param_named(userset, omnibook_userset, int, S_IRUGO);
MODULE_PARM_DESC(ectype, "Type of embedded controller firmware");
MODULE_PARM_DESC(userset, "Use 0 to disable, 1 to enable users to set parameters");


/* End of file */
