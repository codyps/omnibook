/*
 * acpi.c -- ACPI methods low-level access code for TSM30X class laptops
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
#include "ec.h"

#ifdef CONFIG_ACPI

#include <acpi/acpi_drivers.h>

/*
 * ACPI backend masks and strings
 */

#define EC_ACPI_DEVICE "\\_SB.PCI0.LPCB.EC0"

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

/*
 * Probe for expected ACPI device
 * FIXME we check only the ACPI device and not the associated methods
 */
static int omnibook_acpi_probe(const struct omnibook_operation *io_op)
{
	acpi_handle dev_handle;	
	int retval = 0;
	
	if(acpi_disabled) {
		printk(O_ERR "ACPI is disabled: feature unavailable.\n");
		return -ENODEV;
	}

	if(acpi_get_handle(NULL, EC_ACPI_DEVICE, &dev_handle) != AE_OK) {
		printk(O_ERR "Can't get handle on ACPI EC device");	
		return -ENODEV;
	}

	acpi_backend.data = dev_handle;

	dprintk("ACPI probing was successfull\n");
	return retval;
}

/*
 * Execute an ACPI method which return either integer or nothing
 * (acpi_evaluate_object wrapper)
 */
static int omnibook_acpi_execute(char *method, const int *param, int *result)
{

	struct acpi_object_list args_list;
	struct acpi_buffer buff;
	union acpi_object arg, out_objs[1];

	if(!(acpi_backend.data)) {
		dprintk("no handle on EC ACPI device");	
		return -ENODEV;
	}
	
	if(param) {
		args_list.count = 1;
		args_list.pointer = &arg;
		arg.type = ACPI_TYPE_INTEGER;
		arg.integer.value = *param;
	} else
		args_list.count = 0;
	
	buff.length = sizeof(out_objs);
        buff.pointer = out_objs;
	
	if(acpi_evaluate_object(acpi_backend.data, method, &args_list, &buff) != AE_OK) {
		printk(O_ERR "ACPI method execution failed\n");
		return -EIO;
	}
	
	if(!result) /* We don't care what the method returned here */
		return 0;
	
	if(out_objs[0].type != ACPI_TYPE_INTEGER) {
		printk(O_ERR "ACPI method result is not a number\n");
		return -EINVAL;
	}

	*result = out_objs[0].integer.value;
	return 0;
}

static int omnibook_acpi_get_wireless(const struct omnibook_operation *io_op, unsigned int *state)
{
	int retval = 0;
	int raw_state;
	
	if ((retval = omnibook_acpi_execute(GET_WIRELESS_METHOD,0,&raw_state)))
		return retval;


	*state = ( raw_state & WLEX_MASK ) ? WIFI_EX : 0;
	*state |= ( raw_state & WLAT_MASK ) ? WIFI_STA : 0;
	*state |= ( raw_state & KLSW_MASK ) ? KILLSWITCH : 0;
	*state |= ( raw_state & BTEX_MASK ) ? BT_EX : 0;
	*state |= ( raw_state & BTAT_MASK ) ? BT_STA : 0;
	
	return retval;
}

static int omnibook_acpi_set_wireless(const struct omnibook_operation *io_op, unsigned int state)
{
	int retval = 0;
	int raw_state;
	
	raw_state = state & WIFI_STA; /* bit 0 */
	raw_state |= (state & BT_STA) << 0x1; /* bit 1 */

	dprintk("set_wireless raw_state: %x\n", raw_state);

	if ((retval = omnibook_acpi_execute(SET_WIRELESS_METHOD,&raw_state,NULL)))
		return retval;
	
	return retval;
}

static int omnibook_acpi_get_display(const struct omnibook_operation *io_op, unsigned int *state)
{
	int retval = 0;
	int raw_state = 0;

	retval = omnibook_acpi_execute(GET_DISPLAY_METHOD, 0 , &raw_state);
	if(retval < 0)
		return retval;

	/* Backend specific to backend-neutral conversion */
	*state =  ( raw_state & LCD_CSTE) ? DISPLAY_LCD_ON : 0; 
	*state |= ( raw_state & CRT_CSTE) ? DISPLAY_CRT_ON : 0;
	*state |= ( raw_state & TVO_CSTE) ? DISPLAY_TVO_ON : 0; 

	*state |= ( raw_state & LCD_CADL) ? DISPLAY_LCD_DET : 0;
	*state |= ( raw_state & CRT_CADL) ? DISPLAY_CRT_DET : 0;
	*state |= ( raw_state & TVO_CADL) ? DISPLAY_TVO_DET : 0;
	

	return DISPLAY_LCD_ON|DISPLAY_CRT_ON|DISPLAY_TVO_ON|
		DISPLAY_LCD_DET|DISPLAY_CRT_DET|DISPLAY_TVO_DET;
}

static const unsigned int acpi_display_mode_list[] = {
	DISPLAY_LCD_ON,
	DISPLAY_CRT_ON,
	DISPLAY_LCD_ON|DISPLAY_CRT_ON,
	DISPLAY_TVO_ON,
	DISPLAY_LCD_ON|DISPLAY_TVO_ON,
	DISPLAY_CRT_ON|DISPLAY_TVO_ON,
	DISPLAY_LCD_ON|DISPLAY_CRT_ON|DISPLAY_TVO_ON,
};

static int omnibook_acpi_set_display(const struct omnibook_operation *io_op, unsigned int state)
{
	int retval = 0;
	int i, matched;

	for(i = 0; i < ARRAY_SIZE(acpi_display_mode_list); i++) {
		if(acpi_display_mode_list[i] == state) {
			matched = i + 1; /* raw state is array row number + 1 */
			break;
		}	
	}
	if(!matched) {
		printk("Display mode %x is unsupported.\n", state);
		return -EINVAL;
	}
	
	retval = omnibook_acpi_execute(SET_DISPLAY_METHOD, &matched, NULL );
	if(retval < 0)
		return retval;

	return DISPLAY_LCD_ON|DISPLAY_CRT_ON|DISPLAY_TVO_ON;
}

struct omnibook_backend acpi_backend = {
	.name = "acpi",
	.init = omnibook_acpi_probe,
	.aerial_get = omnibook_acpi_get_wireless,
	.aerial_set = omnibook_acpi_set_wireless,
	.display_get = omnibook_acpi_get_display,
	.display_set = omnibook_acpi_set_display,
};

#else /* CONFIG_ACPI */

/* dummy backend for non-ACPI systems */
static int _fail_probe(const struct omnibook_operation *io_op)
{
	return -ENODEV;
}

struct omnibook_backend acpi_backend = {
        .name = "acpi",
        .init = _fail_probe,
};

#endif /* CONFIG_ACPI */
