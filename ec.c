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

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/config.h>
#include <linux/spinlock.h>
#include <linux/acpi.h>
#include <linux/version.h>

#include <asm/io.h>
#include "ec.h"

/*
 * For compatibility with kernel older than 2.6.11
 */

#ifndef DEFINE_SPINLOCK
#define DEFINE_SPINLOCK(s)              spinlock_t s = SPIN_LOCK_UNLOCKED
#endif

extern int omnibook_ectype;

/*
 *	Interrupt control
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

#define OMNIBOOK_EC_STAT_OBF		0x01    /* Output buffer full */
#define OMNIBOOK_EC_STAT_IBF		0x02    /* Input buffer full */
#define OMNIBOOK_EC_STAT_CMD		0x08    /* Last write was a command write (0=data) */

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
	if (timeout>0)
		return 0;
	return -ETIME;
}

/*
 * Read from the embedded controller
 * Decide at run-time if we can use the much cleaner ACPI EC driver instead of
 * this implementation, this is the case if ACPI has been compiled and is not
 * disabled.
 */

int omnibook_ec_read(u8 addr, u8 *data)
{
	unsigned long flags;
	int retval;

#ifdef CONFIG_ACPI_EC
	if (likely(!acpi_disabled))
		return ec_read(addr, data);
#endif
	spin_lock_irqsave(&omnibook_ec_lock, flags);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_IBF);
	if (retval)
		goto end;
	outb(OMNIBOOK_EC_CMD_READ, OMNIBOOK_EC_SC);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_IBF);
	if (retval)
		goto end;
	outb(addr, OMNIBOOK_EC_DATA);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_OBF);
	if (retval)
		goto end;
	*data = inb(OMNIBOOK_EC_DATA);
end:
	spin_unlock_irqrestore(&omnibook_ec_lock, flags);
	return retval;
}

/*
 * Write to the embedded controller
 * Decide at run-time if we can use the much cleaner ACPI EC driver instead of
 * this implementation, this is the case if ACPI has been compiled and is not
 * disabled.
 */

int omnibook_ec_write(u8 addr, u8 data)
{

	unsigned long flags;
	int retval;
	
#ifdef CONFIG_ACPI_EC
	if (likely(!acpi_disabled))
		return ec_write(addr, data);		
#endif
	
	spin_lock_irqsave(&omnibook_ec_lock, flags);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_IBF);
	if (retval)
		goto end;
	outb(OMNIBOOK_EC_CMD_WRITE, OMNIBOOK_EC_SC);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_IBF);
	if (retval)
		goto end;
	outb(addr, OMNIBOOK_EC_DATA);
	retval = omnibook_ec_wait(OMNIBOOK_EC_STAT_IBF);
	if (retval)
		goto end;
	outb(data, OMNIBOOK_EC_DATA);
end:
	spin_unlock_irqrestore(&omnibook_ec_lock, flags);
	return retval;
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
	unsigned long flags;
	int retval;

	spin_lock_irqsave(&omnibook_ec_lock, flags);
	retval = omnibook_kbc_wait(OMNIBOOK_KBC_STAT_IBF);
	if (retval)
		goto end;
	outb(cmd, OMNIBOOK_KBC_SC);
	retval = omnibook_kbc_wait(OMNIBOOK_KBC_STAT_IBF);
      end:
	spin_unlock_irqrestore(&omnibook_ec_lock, flags);
	return retval;
}

/*
 * Write to the keyboard data register
 */

static int omnibook_kbc_write_data(u8 data)
{
	unsigned long flags;
	int retval;

	spin_lock_irqsave(&omnibook_ec_lock, flags);
	retval = omnibook_kbc_wait(OMNIBOOK_KBC_STAT_IBF);
	if (retval)
		goto end;;
	outb(data, OMNIBOOK_KBC_DATA);
	retval = omnibook_kbc_wait(OMNIBOOK_KBC_STAT_IBF);
      end:
	spin_unlock_irqrestore(&omnibook_ec_lock, flags);
	return retval;
}

/*
 * Send a command to keyboard controller
 */

int omnibook_kbc_command(u8 cmd, u8 data)
{
	int retval;

	retval = omnibook_kbc_write_command(cmd);
	if (retval)
		return retval;
	retval = omnibook_kbc_write_data(data);
	return retval;
}

/*
 * Common pattern selector:
 * Match an ectype and return pointer to corresponding io_operation.
 * Also make corresponding backend initialisation if necessary
 */
void *omnibook_io_match(const struct omnibook_io_operation *io_op)
{
	int i;
	void *matched = NULL;
	for (i = 0; io_op[i].ectypes ; i++) {
		if (omnibook_ectype & io_op[i].ectypes) {
			if (io_op[i].type == CDI && omnibook_cdimode_init())
				continue;
			matched = (void *) &io_op[i];
			break;
		}
	}
	return matched;
}

/*
 * Simple read function for common pattern
 * Given an io_operation they read an address and apply a mask on the result
 */

int omnibook_io_read(struct omnibook_io_operation *io_op, u8 *value)
{
	int retval = 0;
	
	switch(io_op->type) {
	case EC:
		retval = omnibook_ec_read( io_op->read, value);
		break;
	case CDI:
		retval = omnibook_cdimode_read( io_op->read, value);
		break;
	default:
		BUG();
	}
	
	if (io_op->mask)
		*value = *value & io_op->mask;
	
	return retval;		
}

/*
 * Simple write function for common pattern
 * Given an io_operation they write value at an given address
 */

int omnibook_io_write(struct omnibook_io_operation *io_op, u8 value)
{
	int retval = 0;
	
	switch(io_op->type) {
	case EC:
		retval = omnibook_ec_write( io_op->write, value);
		break;
	case CDI:
		retval = omnibook_cdimode_write( io_op->write, value);
		break;
	default:
		BUG();
	}
	
	return retval;		
}

/* End of file */
