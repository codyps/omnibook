/*
 * acpi.c -- ACPI methods low-level access code for TSM70 class laptops
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * Written by Mathieu Bérard <mathieu.berard@crans.org>, 2006
 *
 */

#include "omnibook.h"
#include "hardware.h"

#ifdef CONFIG_ACPI

#include <acpi/acpi_drivers.h>

/*
 * ACPI backend masks and strings
 */

#define GET_WIRELESS_METHOD "ANTR"
#define SET_WIRELESS_METHOD "ANTW"
#define WLEX_MASK 	0x4
#define WLAT_MASK	0x1
#define BTEX_MASK	0x8
#define BTAT_MASK	0x2
#define KLSW_MASK	0x10

#define GET_DISPLAY_METHOD "DOSS"
#define SET_DISPLAY_METHOD "DOSW"
/* Display reading masks CADL = detected, CSTE = enabled */
#define	LCD_CADL	0x10
#define	CRT_CADL	0x20
#define	TVO_CADL	0x40
#define	LCD_CSTE	0x1
#define	CRT_CSTE	0x2
#define	TVO_CSTE	0x4

static char ec_dev_list[][20] = {
	"\\_SB.PCI0.LPCB.EC0",
	"\\_SB.PCI0.LPC0.EC0",
};

#define TOSHIBA_ACPI_BT_CLASS "bluetooth"
#define TOSHIBA_ACPI_DEVICE_NAME "bluetooth adapter"

#define TOSH_BT_ACTIVATE_USB "AUSB"
#define TOSH_BT_DISABLE_USB  "DUSB"
#define TOSH_BT_POWER_ON     "BTPO"
#define TOSH_BT_POWER_OFF    "BTPF"
#define TOSH_BT_STATUS	     "BTST"
#define	TOSH_BT_KSST_MASK	0x1
#define	TOSH_BT_USB_MASK	0x40
#define	TOSH_BT_POWER_MASK	0x80

/*
 * ACPI driver for Toshiba Bluetooth device
 */
static int omnibook_acpi_bt_add(struct acpi_device *device);
static int omnibook_acpi_bt_remove(struct acpi_device *device, int type);

static struct acpi_driver omnibook_bt_driver = {
	.name	= OMNIBOOK_MODULE_NAME,
	.class	= TOSHIBA_ACPI_BT_CLASS,
	.ids	= "TOS6205",
	.ops	= {
			.add	=  omnibook_acpi_bt_add,
			.remove	=  omnibook_acpi_bt_remove,
		  },
};

/*
 * ACPI backend private data structure
 */
struct acpi_backend_data {
	acpi_handle ec_handle;  /* Handle on ACPI EC device */
	acpi_handle bt_handle;  /* Handle on ACPI BT device */
	unsigned has_antr_antw:1; /* Are there ANTR/ANTW methods in the EC device ? */
};

/*
 * Probe for expected ACPI devices
 */
static int omnibook_acpi_init(const struct omnibook_operation *io_op)
{
	int retval = 0;	
	acpi_handle dev_handle, method_handle;
	int i;
	struct acpi_backend_data *priv_data;
	
	if (unlikely(acpi_disabled)) {
		printk(O_ERR "ACPI is disabled: feature unavailable.\n");
		return -ENODEV;
	}

	if (!io_op->backend->data) {
		dprintk("Try to init ACPI backend\n");
		mutex_init(&io_op->backend->mutex);
		mutex_lock(&io_op->backend->mutex);
		kref_init(&io_op->backend->kref);
		priv_data = kzalloc(sizeof(struct acpi_backend_data), GFP_KERNEL);
		if (!priv_data) {
			retval = -ENOMEM;
			goto error0;
		}

		/* Locate ACPI EC device, acpi_get_handle set dev_handle to NULL if not found */
		for (i = 0; i < ARRAY_SIZE(ec_dev_list); i++) {
			if (acpi_get_handle(NULL, ec_dev_list[i], &dev_handle) == AE_OK) {
				dprintk("ACPI EC device found\n");
				priv_data->ec_handle = dev_handle;
				break;
			}
		}
		
		if(!dev_handle) {
			printk(O_ERR "Can't get handle on ACPI EC device.\n");
			retval = -ENODEV;
			goto error1;
		}

		if((acpi_get_handle( dev_handle, "ANTR", &method_handle) == AE_OK) &&
		    (acpi_get_handle( dev_handle, "ANTW", &method_handle) == AE_OK))
			priv_data->has_antr_antw = 1;

		io_op->backend->data = (void *) priv_data;
		
		/* attempt to register Toshiba bluetooth ACPI driver */
		acpi_bus_register_driver(&omnibook_bt_driver);

		dprintk("ACPI backend init OK\n");
		mutex_unlock(&io_op->backend->mutex);
		return 0;

	} else {
		dprintk("ACPI backend has already been initialized\n");
		kref_get(&io_op->backend->kref);
		return 0;
	}
		
	error1:
	kfree(priv_data);
	io_op->backend->data = NULL;
	error0:
	mutex_unlock(&io_op->backend->mutex);
	mutex_destroy(&io_op->backend->mutex);
	return retval;
}

static void omnibook_acpi_free(struct kref *ref)
{
	struct omnibook_backend *backend;
	backend = container_of(ref, struct omnibook_backend, kref);
	dprintk("ACPI backend not used anymore: disposing\n");
	acpi_bus_unregister_driver(&omnibook_bt_driver);
	kfree(backend->data);
}

static void omnibook_acpi_exit(const struct omnibook_operation *io_op)
{
	dprintk("Trying to dispose ACPI backend\n");
	kref_put(&io_op->backend->kref, omnibook_acpi_free);
}

/* forward declaration */
struct omnibook_backend acpi_backend;

/*
 * Execute an ACPI method which return either an integer or nothing
 * (acpi_evaluate_object wrapper)
 */
static int omnibook_acpi_execute(acpi_handle dev_handle, char *method, const int *param, int *result)
{

	struct acpi_object_list args_list;
	struct acpi_buffer buff;
	union acpi_object arg, out_objs[1];
	
	WARN_ON(!mutex_is_locked(&acpi_backend.mutex));

	if (param) {
		args_list.count = 1;
		args_list.pointer = &arg;
		arg.type = ACPI_TYPE_INTEGER;
		arg.integer.value = *param;
	} else
		args_list.count = 0;

	buff.length = sizeof(out_objs);
	buff.pointer = out_objs;

	if (acpi_evaluate_object(dev_handle, method, &args_list, &buff) != AE_OK) {
		printk(O_ERR "ACPI method execution failed\n");
		return -EIO;
	}

	if (!result)		/* We don't care what the method returned here */
		return 0;

	if (out_objs[0].type != ACPI_TYPE_INTEGER) {
		printk(O_ERR "ACPI method result is not a number\n");
		return -EINVAL;
	}

	*result = out_objs[0].integer.value;
	return 0;
}

/*
 * Set Bluetooth device state using the Toshiba BT device
 */
static int set_bt_status(const struct acpi_backend_data *priv_data, unsigned int state)
{
	int retval = 0;

	if(state) {
		retval = omnibook_acpi_execute(priv_data->bt_handle, TOSH_BT_ACTIVATE_USB, NULL, NULL);
		if(retval)
			goto out;
		retval = omnibook_acpi_execute(priv_data->bt_handle, TOSH_BT_POWER_ON, NULL, NULL);
		if(retval)
			goto out;
	} else {
		retval = omnibook_acpi_execute(priv_data->bt_handle, TOSH_BT_DISABLE_USB, NULL, NULL);
		if(retval)
			goto out;
		retval = omnibook_acpi_execute(priv_data->bt_handle, TOSH_BT_POWER_OFF, NULL, NULL);
		if(retval)
			goto out;
	}
	out:
	return retval;
}

static int omnibook_acpi_bt_add(struct acpi_device *device)
{
	struct acpi_backend_data *priv_data = acpi_backend.data;
	
	dprintk("Enabling found Toshiba Bluetooth ACPI device.\n");
	strcpy(acpi_device_name(device), TOSHIBA_ACPI_DEVICE_NAME);
        strcpy(acpi_device_class(device), TOSHIBA_ACPI_BT_CLASS);

	/* Save handle in backend private data structure. ugly. */
	priv_data->bt_handle = device->handle;

	return set_bt_status(priv_data, 1);
}

static int omnibook_acpi_bt_remove(struct acpi_device *device, int type)
{
	int retval;	
	struct acpi_backend_data *priv_data = acpi_backend.data;	

	dprintk("Disabling Toshiba Bluetooth ACPI device.\n");
	retval = set_bt_status(priv_data, 0);
	priv_data->bt_handle = NULL;
	return retval;
}

/*
 * Get Bluetooth status using the BTST method
 */
static int get_bt_status(const struct acpi_backend_data *priv_data, unsigned int *state)
{
	int retval = 0;
	int raw_state;

	if ((retval = omnibook_acpi_execute(priv_data->bt_handle, TOSH_BT_STATUS, 0, &raw_state)))
		return retval;

	dprintk("BTST raw_state: %x\n", raw_state);

	*state = BT_EX;
	*state |= ((raw_state & TOSH_BT_USB_MASK) && (raw_state & TOSH_BT_POWER_MASK)) ? BT_STA : 0;

	return retval;
}

/*
 * Get the Bluetooth + Wireless status using the ANTR method
 * FIXME: what if ANTR and BTST disagree ? we thrust ANTR for now
 */
static int get_wireless_status(const struct acpi_backend_data *priv_data, unsigned int *state)
{
	int retval = 0;
	int raw_state;

	if ((retval = omnibook_acpi_execute(priv_data->ec_handle, GET_WIRELESS_METHOD, 0, &raw_state)))
		return retval;

	dprintk("get_wireless raw_state: %x\n", raw_state);

	*state = (raw_state & WLEX_MASK) ? WIFI_EX : 0;
	*state |= (raw_state & WLAT_MASK) ? WIFI_STA : 0;
	*state |= (raw_state & KLSW_MASK) ? KILLSWITCH : 0;
	*state |= (raw_state & BTEX_MASK) ? BT_EX : 0;
	*state |= (raw_state & BTAT_MASK) ? BT_STA : 0;

	return retval;
}

static int omnibook_acpi_get_wireless(const struct omnibook_operation *io_op, unsigned int *state)
{
	int retval;
	struct acpi_backend_data *priv_data = io_op->backend->data;

	/* use BTST (BT device) if we don't have ANTR/ANTW (EC device) */
	if(priv_data->has_antr_antw)
		retval = get_wireless_status(priv_data, state);
	else if(priv_data->bt_handle)
		retval = get_bt_status(priv_data, state);
	else
		retval = -ENODEV;

	return retval;
}

/*
 * Set the Bluetooth + Wireless status using the ANTW method
 */
static int set_wireless_status(const struct acpi_backend_data *priv_data, unsigned int state)
{
	int retval;
	int raw_state;

	raw_state = !!(state & WIFI_STA);	/* bit 0 */
	raw_state |= !!(state & BT_STA) << 0x1;	/* bit 1 */

	dprintk("set_wireless raw_state: %x\n", raw_state);

	retval = omnibook_acpi_execute(priv_data->ec_handle, SET_WIRELESS_METHOD, &raw_state, NULL);

	return retval;
}

static int omnibook_acpi_set_wireless(const struct omnibook_operation *io_op, unsigned int state)
{
	int retval = -ENODEV;
	struct acpi_backend_data *priv_data = io_op->backend->data;
	
	/* First try the ANTR/ANTW methods */
	if(priv_data->has_antr_antw)
		retval = set_wireless_status(priv_data, state);	
	
	/* Then try the bluetooth ACPI device if present */
	if(priv_data->bt_handle)
		retval = set_bt_status(priv_data, (state & BT_STA));

	return retval;
}

static int omnibook_acpi_get_display(const struct omnibook_operation *io_op, unsigned int *state)
{
	int retval = 0;
	int raw_state = 0;
	struct acpi_backend_data *priv_data = io_op->backend->data;
	
	retval = omnibook_acpi_execute(priv_data->ec_handle, GET_DISPLAY_METHOD, 0, &raw_state);
	if (retval < 0)
		return retval;

	dprintk("get_display raw_state: %x\n", raw_state);

	/* Backend specific to backend-neutral conversion */
	*state = (raw_state & LCD_CSTE) ? DISPLAY_LCD_ON : 0;
	*state |= (raw_state & CRT_CSTE) ? DISPLAY_CRT_ON : 0;
	*state |= (raw_state & TVO_CSTE) ? DISPLAY_TVO_ON : 0;

	*state |= (raw_state & LCD_CADL) ? DISPLAY_LCD_DET : 0;
	*state |= (raw_state & CRT_CADL) ? DISPLAY_CRT_DET : 0;
	*state |= (raw_state & TVO_CADL) ? DISPLAY_TVO_DET : 0;

	return DISPLAY_LCD_ON | DISPLAY_CRT_ON | DISPLAY_TVO_ON | DISPLAY_LCD_DET | DISPLAY_CRT_DET
	    | DISPLAY_TVO_DET;
}

static const unsigned int acpi_display_mode_list[] = {
	DISPLAY_LCD_ON,
	DISPLAY_CRT_ON,
	DISPLAY_LCD_ON | DISPLAY_CRT_ON,
	DISPLAY_TVO_ON,
	DISPLAY_LCD_ON | DISPLAY_TVO_ON,
	DISPLAY_CRT_ON | DISPLAY_TVO_ON,
	DISPLAY_LCD_ON | DISPLAY_CRT_ON | DISPLAY_TVO_ON,
};

static int omnibook_acpi_set_display(const struct omnibook_operation *io_op, unsigned int state)
{
	int retval = 0;
	int i; 
	int matched = -1;
	struct acpi_backend_data *priv_data = io_op->backend->data;

	for (i = 0; i < ARRAY_SIZE(acpi_display_mode_list); i++) {
		if (acpi_display_mode_list[i] == state) {
			matched = i + 1;	/* raw state is array row number + 1 */
			break;
		}
	}
	if (matched == -1) {
		printk("Display mode %x is unsupported.\n", state);
		return -EINVAL;
	}

	dprintk("set_display raw_state: %x\n", matched);

	retval = omnibook_acpi_execute(priv_data->ec_handle, SET_DISPLAY_METHOD, &matched, NULL);
	if (retval < 0)
		return retval;

	return DISPLAY_LCD_ON | DISPLAY_CRT_ON | DISPLAY_TVO_ON;
}

struct omnibook_backend acpi_backend = {
	.name = "acpi",
	.init = omnibook_acpi_init,
	.exit = omnibook_acpi_exit,
	.aerial_get = omnibook_acpi_get_wireless,
	.aerial_set = omnibook_acpi_set_wireless,
	.display_get = omnibook_acpi_get_display,
	.display_set = omnibook_acpi_set_display,
};

#else				/* CONFIG_ACPI */

/* dummy backend for non-ACPI systems */
static int _fail_probe(const struct omnibook_operation *io_op)
{
	return -ENODEV;
}

struct omnibook_backend acpi_backend = {
	.name = "acpi",
	.init = _fail_probe,
};

#endif				/* CONFIG_ACPI */
