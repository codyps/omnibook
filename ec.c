/*
 * ec.c -- low level functions to access Embedded Controller,
 *         Keyboard Controller and system I/O ports or memory
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

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/ioport.h>

#include <asm/io.h>
#include "ec.h"
#include "compat.h"

/*
 * Interrupt control
 */

static DEFINE_SPINLOCK(omnibook_ec_lock);

/*
 * Timeout in ms for sending to controller
 */

#define OMNIBOOK_TIMEOUT                250

/*
 * Registers of the embedded controller
 */

#define OMNIBOOK_EC_DATA		0x62
#define OMNIBOOK_EC_SC			0x66

/*
 * Embedded controller status register bits
 */

#define OMNIBOOK_EC_STAT_OBF		0x01	/* Output buffer full */
#define OMNIBOOK_EC_STAT_IBF		0x02	/* Input buffer full */
#define OMNIBOOK_EC_STAT_CMD		0x08	/* Last write was a command write (0=data) */

/*
 * Embedded controller commands
 */

#define OMNIBOOK_EC_CMD_READ		0x80
#define OMNIBOOK_EC_CMD_WRITE		0x81
#define OMNIBOOK_EC_CMD_QUERY		0x84

/*
 * Wait for embedded controller buffer
 */

static int omnibook_ec_wait(u8 event)
{
	int timeout = OMNIBOOK_TIMEOUT;

	switch (event) {
	case OMNIBOOK_EC_STAT_OBF:
		while (!(inb(OMNIBOOK_EC_SC) & event) && timeout--)
			mdelay(1);
		break;
	case OMNIBOOK_EC_STAT_IBF:
		while ((inb(OMNIBOOK_EC_SC) & event) && timeout--)
			mdelay(1);
		break;
	default:
		return -EINVAL;
	}
	if (timeout > 0)
		return 0;
	return -ETIME;
}

/*
 * Read from the embedded controller
 * Decide at run-time if we can use the much cleaner ACPI EC driver instead of
 * this implementation, this is the case if ACPI has been compiled and is not
 * disabled.
 */

static int omnibook_ec_read(const struct omnibook_operation *io_op, u8 * data)
{
	int retval;

#ifdef CONFIG_ACPI_EC
	if (likely(!acpi_disabled)) {
		retval = ec_read((u8) io_op->read_addr, data);
		if (io_op->read_mask)
			*data &= io_op->read_mask;
		dprintk("ACPI EC read at %lx success %i.\n", io_op->read_addr, retval);
		return retval;
	}
#endif
	spin_lock_irq(&omnibook_ec_lock);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_IBF);
	if (retval)
		goto end;
	outb(OMNIBOOK_EC_CMD_READ, OMNIBOOK_EC_SC);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_IBF);
	if (retval)
		goto end;
	outb((u8) io_op->read_addr, OMNIBOOK_EC_DATA);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_OBF);
	if (retval)
		goto end;
	*data = inb(OMNIBOOK_EC_DATA);
	if (io_op->read_mask)
		*data &= io_op->read_mask;
      end:
	spin_unlock_irq(&omnibook_ec_lock);
	dprintk("Custom EC read at %lx success %i.\n", io_op->read_addr, retval);
	return retval;
}

/*
 * Write to the embedded controller:
 * If OMNIBOOK_LEGACY is set, decide at run-time if we can use the much cleaner 
 * ACPI EC driver instead of this legacy implementation. 
 * This is the case if ACPI has been compiled and is not
 * disabled.
 * If OMNIBOOK_LEGACY is unset, we drop our custoim implementation
 */

static int omnibook_ec_write(const struct omnibook_operation *io_op, u8 data)
{
	int retval;

#ifdef CONFIG_ACPI_EC
	if (likely(!acpi_disabled)) {
		retval = ec_write((u8) io_op->write_addr, data);
		dprintk("ACPI EC write at %lx success %i.\n", io_op->write_addr, retval);
		return retval;
	}
#endif

	spin_lock_irq(&omnibook_ec_lock);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_IBF);
	if (retval)
		goto end;
	outb(OMNIBOOK_EC_CMD_WRITE, OMNIBOOK_EC_SC);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_IBF);
	if (retval)
		goto end;
	outb((u8) io_op->write_addr, OMNIBOOK_EC_DATA);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_IBF);
	if (retval)
		goto end;
	outb(data, OMNIBOOK_EC_DATA);
      end:
	spin_unlock_irq(&omnibook_ec_lock);
	dprintk("Custom EC write at %lx success %i.\n", io_op->write_addr, retval);
	return retval;
}

/*
 * legacy access function for unconverted old code who expect old omnibook_ec_read
 */

int legacy_ec_read(u8 addr, u8 * data)
{
	int retval;
	struct omnibook_operation io_op = SIMPLE_BYTE(EC, addr, 0);
	retval = omnibook_ec_read(&io_op, data);
	return retval;
}

/*
 * legacy access function for unconverted old code who expect old omnibook_ec_write
 */

int legacy_ec_write(u8 addr, u8 data)
{
	int retval;
	struct omnibook_operation io_op = SIMPLE_BYTE(EC, addr, 0);
	retval = omnibook_ec_write(&io_op, data);
	return retval;
}

static int omnibook_ec_display(const struct omnibook_operation *io_op, unsigned int *state)
{
	int retval;
	u8 raw_state;

	retval = io_op->backend->byte_read(io_op, &raw_state);
	if (retval < 0)
		return retval;

	*state = !!(raw_state) & DISPLAY_CRT_DET;

	return DISPLAY_CRT_DET;
}

/*
 * Registers of the keyboard controller
 */

#define OMNIBOOK_KBC_DATA		0x60
#define OMNIBOOK_KBC_SC			0x64

/*
 * Keyboard controller status register bits
 */

#define OMNIBOOK_KBC_STAT_OBF		0x01	/* Output buffer full */
#define OMNIBOOK_KBC_STAT_IBF		0x02	/* Input buffer full */
#define OMNIBOOK_KBC_STAT_CMD		0x08	/* Last write was a command write (0=data) */

/*
 * Wait for keyboard buffer
 */

static int omnibook_kbc_wait(u8 event)
{
	int timeout = OMNIBOOK_TIMEOUT;

	switch (event) {
	case OMNIBOOK_KBC_STAT_OBF:
		while (!(inb(OMNIBOOK_KBC_SC) & event) && timeout--)
			mdelay(1);
		break;
	case OMNIBOOK_KBC_STAT_IBF:
		while ((inb(OMNIBOOK_KBC_SC) & event) && timeout--)
			mdelay(1);
		break;
	default:
		return -EINVAL;
	}
	if (timeout > 0)
		return 0;
	return -ETIME;
}

/*
 * Write to the keyboard command register
 */

static int omnibook_kbc_write_command(u8 cmd)
{
	int retval;

	spin_lock_irq(&omnibook_ec_lock);
	retval = omnibook_kbc_wait(OMNIBOOK_KBC_STAT_IBF);
	if (retval)
		goto end;
	outb(cmd, OMNIBOOK_KBC_SC);
	retval = omnibook_kbc_wait(OMNIBOOK_KBC_STAT_IBF);
      end:
	spin_unlock_irq(&omnibook_ec_lock);
	return retval;
}

/*
 * Write to the keyboard data register
 */

static int omnibook_kbc_write_data(u8 data)
{
	int retval;

	spin_lock_irq(&omnibook_ec_lock);
	retval = omnibook_kbc_wait(OMNIBOOK_KBC_STAT_IBF);
	if (retval)
		goto end;;
	outb(data, OMNIBOOK_KBC_DATA);
	retval = omnibook_kbc_wait(OMNIBOOK_KBC_STAT_IBF);
      end:
	spin_unlock_irq(&omnibook_ec_lock);
	return retval;
}

/*
 * Send a command to keyboard controller
 */

static int omnibook_kbc_command(const struct omnibook_operation *io_op, u8 data)
{
	int retval;

	if ((retval = omnibook_kbc_write_command(OMNIBOOK_KBC_CONTROL_CMD)))
		return retval;

	retval = omnibook_kbc_write_data(data);
	return retval;
}

/*
 * Onetouch button hotkey handler
 */
static int omnibook_kbc_hotkeys(const struct omnibook_operation *io_op, unsigned int state)
{
	int retval;

	retval = omnibook_toggle(io_op, !!(state & HKEY_ONETOUCH));

	if (retval < 0)
		return retval;
	else
		return HKEY_ONETOUCH;
}

/*
 * IO port backend. Only support single or dual ports operations
 * private data structure: it's the linked list of requested ports
 * 
 * Race condition issue: omnibook_pio_init/exit functions are only called from
 * omnibook_backend_match and omnibook_remove from init.c, this should happen
 * only at module init/exit time so there is no need for a lock.
 */

struct pio_private_data_t {
	unsigned long addr;
	struct kref refcount;
	struct list_head list;
};

static struct pio_private_data_t pio_private_data = {
	.addr = 0,
	.list = LIST_HEAD_INIT(pio_private_data.list),
};

/*
 * Match an entry in the linked list helper function: see if we have and entry
 * whose addr field match maddr
 */
static struct pio_private_data_t *omnibook_match_port(struct pio_private_data_t *data,
						      unsigned long maddr)
{
	struct list_head *p;
	struct pio_private_data_t *cursor;

	list_for_each(p, &data->list) {
		cursor = list_entry(p, struct pio_private_data_t, list);
		if (cursor->addr == maddr) {
			return cursor;
		}
	}
	return NULL;
}

/*
 * See if we have to request raddr
 */
static int omnibook_claim_port(struct pio_private_data_t *data, unsigned long raddr)
{
	struct pio_private_data_t *match, *new;

	match = omnibook_match_port(data, raddr);
	if (match) {
		/* Already requested by us: increment kref and quit */
		kref_get(&match->refcount);
		return 0;
	}

	/* there was no match: request the region and add to list */
	if (!request_region(raddr, 1, OMNIBOOK_MODULE_NAME)) {
		printk(O_ERR "Request I/O port error\n");
		return -ENODEV;
	}

	new = kmalloc(sizeof(struct pio_private_data_t), GFP_KERNEL);
	if (!new) {
		release_region(raddr, 1);
		return -ENOMEM;
	}

	kref_init(&new->refcount);
	new->addr = raddr;
	list_add(&new->list, &data->list);

	return 0;
}

/*
 * Register read_addr and write_addr
 */
static int omnibook_pio_init(const struct omnibook_operation *io_op)
{
	int retval = 0;

	if (io_op->read_addr
	    && (retval = omnibook_claim_port(io_op->backend->data, io_op->read_addr)))
		goto out;

	if (io_op->write_addr && (io_op->write_addr != io_op->read_addr))
		retval = omnibook_claim_port(io_op->backend->data, io_op->write_addr);

      out:
	return retval;
}

/*
 * REALLY release a port
 */
static void omnibook_free_port(struct kref *ref)
{
	struct pio_private_data_t *data;

	data = container_of(ref, struct pio_private_data_t, refcount);
	release_region(data->addr, 1);
	list_del(&data->list);
	kfree(data);
}

/*
 * Unregister read_addr and write_addr
 */
static void omnibook_pio_exit(const struct omnibook_operation *io_op)
{
	struct pio_private_data_t *match;

	match = omnibook_match_port(io_op->backend->data, io_op->read_addr);
	if (match)
		kref_put(&match->refcount, omnibook_free_port);

	match = NULL;
	match = omnibook_match_port(io_op->backend->data, io_op->write_addr);
	if (match)
		kref_put(&match->refcount, omnibook_free_port);

}

static int omnibook_io_read(const struct omnibook_operation *io_op, u8 * value)
{
	*value = inb(io_op->read_addr);
	if (io_op->read_mask)
		*value &= io_op->read_mask;
	return 0;
}

static int omnibook_io_write(const struct omnibook_operation *io_op, u8 value)
{
	outb(io_op->write_addr, value);
	return 0;
}

/*
 * Backend interface declarations
 */

struct omnibook_backend kbc_backend = {
	.name = "i8042",
	.byte_write = omnibook_kbc_command,
	.hotkeys_set = omnibook_kbc_hotkeys,
};

struct omnibook_backend pio_backend = {
	.name = "pio",
	.data = &pio_private_data,
	.init = omnibook_pio_init,
	.exit = omnibook_pio_exit,
	.byte_read = omnibook_io_read,
	.byte_write = omnibook_io_write,
};

struct omnibook_backend ec_backend = {
	.name = "ec",
	.byte_read = omnibook_ec_read,
	.byte_write = omnibook_ec_write,
	.display_get = omnibook_ec_display,
};

int omnibook_apply_write_mask(const struct omnibook_operation *io_op, int toggle)
{
	int retval = 0;
	int mask;
	u8 data;

	if ((retval = io_op->backend->byte_read(io_op, &data)))
		return retval;

	if (toggle == 1)
		mask = io_op->on_mask;
	else if (toggle == 0)
		mask = io_op->off_mask;
	else
		return -EINVAL;

	if (mask > 0)
		data |= (u8) mask;
	else if (mask < 0)
		data &= ~((u8) (-mask));
	else
		return -EINVAL;

	retval = io_op->backend->byte_write(io_op, data);

	return retval;
}

/*
 * Helper for toggle like operations
 */
int omnibook_toggle(const struct omnibook_operation *io_op, int toggle)
{
	int retval;
	u8 data;

	data = toggle ? io_op->on_mask : io_op->off_mask;
	retval = io_op->backend->byte_write(io_op, data);
	return retval;
}

/* End of file */
