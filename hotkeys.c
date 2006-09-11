/*
 * hotkeys.c -- code to handling Hotkey/E-Key/EasyAccess buttons
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

#include "omnibook.h"
#include "ec.h"


/*
 * Save state for suspend/resume operation
 */
static unsigned int saved_state;

/* Predefined convinient on/off states */
#define HKEY_ON  HKEY_ONETOUCH|HKEY_MULTIMEDIA|HKEY_FN|HKEY_DOCK|HKEY_FNF5
#define HKEY_OFF 0

static int omnibook_hotkeys_set(struct omnibook_operation *io_op, unsigned int state)
{
	int write_capability;

	write_capability = io_op->backend->hotkeys_set(io_op, state);
	if( write_capability < 0)
		goto out;

	/* Update saved state */
	saved_state = state & write_capability;

out:	
	return write_capability;
}

static int omnibook_hotkeys_get(struct omnibook_operation *io_op, unsigned int *state)
{
	
	unsigned int read_state = 0;
	int read_capability = 0;
	
	if(io_op->backend->hotkeys_get)
		read_capability = io_op->backend->hotkeys_get(io_op, &read_state);
	if(read_capability < 0)
		goto out;

	/* Return previously set state for the fields that are write only */
	*state = (read_state & read_capability) + (saved_state & ~read_capability);

out:	
	return read_capability;
}

/*
 * Power management handlers
 */
 
/*
 * Restore previously saved state
 */
static int omnibook_hotkeys_resume(struct omnibook_operation *io_op)
{
	int retval;
	retval = io_op->backend->hotkeys_set(io_op, saved_state);
	return min(retval,0);
}

/*
 * Save state and disable hotkeys upon suspend (FIXME is the disabling required ?)
 */
static int omnibook_hotkeys_suspend(struct omnibook_operation *io_op)
{
	int retval = 0;

	retval = omnibook_hotkeys_get(io_op, &saved_state);
	if(retval < 0)
		return retval;

	retval = io_op->backend->hotkeys_set(io_op, HKEY_OFF);
	if(retval < 0)
		return retval;
	
	return 0;
}

static const char pretty_name[][27] = {
	"Onetouch buttons",
	"Multimedia hotkeys are",
	"Fn hotkeys are",
	"Stick key is",
	"Press Fn twice to lock is",
	"Dock events are",
	"Fn + F5 hotkey is"
};

static int omnibook_hotkeys_read(char *buffer,struct omnibook_operation *io_op)
{
	int len = 0;
	int read_capability, write_capability;
	unsigned int read_state, mask;

	read_capability = omnibook_hotkeys_get(io_op, &read_state);
	if(read_capability < 0)
		return read_capability;

	write_capability = omnibook_hotkeys_set(io_op, read_state);
	if(write_capability < 0)
		return write_capability;

	for( mask = HKEY_ONETOUCH ; mask <= HKEY_FNF5; mask = mask << 1) {
	/* we assume write capability or read capability imply support */
		 if( (read_capability | write_capability ) & mask )
			len += sprintf(buffer + len, "%s %s.\n", 
				pretty_name[ ffs(mask) - 1 ],
				( read_state & mask ) ? "enabled" : "disabled");
	}
	return len;
}

static int omnibook_hotkeys_write(char *buffer,struct omnibook_operation *io_op)
{
	unsigned int state;	
	char *endp;
	
	if (strncmp(buffer, "off", 3) == 0)
		omnibook_hotkeys_set(io_op, HKEY_OFF);
	else if (strncmp(buffer, "on", 2) == 0)
		omnibook_hotkeys_set(io_op, HKEY_ON);
	else {
		state = simple_strtoul(buffer, &endp, 16);
		if (endp == buffer)
			return -EINVAL;
		else
			omnibook_hotkeys_set(io_op, state);
	}
	return 0;
}

static int __init omnibook_hotkeys_init(struct omnibook_operation *io_op)
{	
	int retval;	
	printk(O_INFO "Enabling all hotkeys.\n");
	retval = omnibook_hotkeys_set(io_op, HKEY_ON);
	return retval < 0 ? retval : 0;
}

static void __exit omnibook_hotkeys_cleanup(struct omnibook_operation *io_op)
{
	printk(O_INFO "Disabling all hotkeys.\n");
	omnibook_hotkeys_set(io_op, HKEY_OFF);
}

static struct omnibook_tbl hotkey_table[] __initdata = {
	{ XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|AMILOD|TSP10, {KBC,}},
	{ TSM30X,	{CDI,}},
	{ TSM40,	{SMI,}},
	{ 0,}
};

static struct omnibook_feature __declared_feature hotkeys_driver = {
	 .name = "hotkeys",
	 .enabled = 1,
	 .read = omnibook_hotkeys_read,
	 .write = omnibook_hotkeys_write,
	 .init = omnibook_hotkeys_init,
	 .exit = omnibook_hotkeys_cleanup,
	 .suspend = omnibook_hotkeys_suspend,
	 .resume = omnibook_hotkeys_resume,
	 .ectypes = XE3GF|XE3GC|OB500|OB510|OB6000|OB6100|XE4500|AMILOD|TSP10|TSM30X|TSM40,
	 .tbl = hotkey_table,
};

module_param_named(hotkeys, hotkeys_driver.enabled, int, S_IRUGO);
MODULE_PARM_DESC(hotkeys, "Use 0 to disable, 1 to enable hotkeys handling");
/* End of file */
