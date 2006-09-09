/*
 * nbsmi.c -- Toshiba SMI low-level acces code
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
 * Sources of inspirations for this code were:
 * -Toshiba via provided hardware specification
 * -Thorsten Zachmann with the 's1bl' project
 * -Frederico Munoz with the 'tecra_acpi' project
 * Thanks to them
 */
 
#include "omnibook.h"

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16))
#include <asm/semaphore.h>
#define DEFINE_MUTEX(lock)		DECLARE_MUTEX(lock)
#define mutex_lock(lock)		down(lock)
#define mutex_lock_interruptible(lock)	down_interruptible(lock)
#define mutex_unlock(lock)		up(lock)
#else
#include <linux/mutex.h>
#endif

#include <linux/preempt.h>
#include <linux/pci.h>
#include <linux/kref.h>
#include <asm/io.h>
#include <asm/mc146818rtc.h>
#include "ec.h"

/*
 * ATI's IXP PCI-LPC bridge
 */
#define PCI_DEVICE_ID_ATI_SB400 0x4377

#define INTEL_PMBASE	0x40
#define INTEL_GPE0_EN	0x2c

#define BUFFER_SIZE	0x20
#define INTEL_OFFSET	0x60
#define	INTEL_SMI_PORT	0xb2 /* APM_CNT port in INTEL ICH specs */
#define ATI_OFFSET	0xef
#define	ATI_SMI_PORT	0xb0

#define	EC_INDEX_PORT	0x300
#define EC_DATA_PORT	0x301

/* Masks decode for GetAeral */
#define WLEX_MASK 	0x4
#define WLAT_MASK	0x8
#define BTEX_MASK	0x1
#define BTAT_MASK	0x2

/* 
 * We serialize access to this backend using a mutex
 * Crital sections around #SMI triggering are run atomically using a spinlock 
 */
static DEFINE_MUTEX(smi_lock);
static DEFINE_SPINLOCK(smi_spinlock);

/*
 * Private data of this backend
 */
static struct kref *refcount;
static struct pci_dev *lpc_bridge; 	/* Southbridge chip ISA bridge/LPC interface PCI device */
static u8 start_offset;
static int already_failed = 0;		/* Backend init already failed at leat once */

/*
 * Possible list of supported southbridges
 * Here mostly to implement a more or less clean PCI probing
 * Works only because of previous DMI probing.
 * It's in compal.c
 */
extern const struct pci_device_id lpc_bridge_table[];

/*
 * Since we are going to trigger an SMI, all registers may be mangled in the
 * process. So we save and restore all registers and eflags in the stack.
 * We also disable preemtion and IRQs upon SMI call.
 */
static inline void save_all_regs_flag(void)
{	
	spin_lock_irq(&smi_spinlock);
	__asm__ __volatile__("pushal ; pushfl");
}

static inline void restore_all_regs_flag(void)
{
	__asm__ __volatile__("popfl; popal"
			     :
			     : 
			     : "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi");
	spin_unlock_irq(&smi_spinlock);	
}

static inline void ati_do_smi_call(int *retval, u16 function)
{
	outw( function, ATI_SMI_PORT ); /* Call to SMI */
	*retval = inw(ATI_SMI_PORT + 1);
}

static inline void intel_do_smi_call(int *retval, u16 function, u32 sci_en)
{
	u32 state;
	
	state = inl(sci_en);
	outl( 0, sci_en );

	outw( function, INTEL_SMI_PORT ); /* Call to SMI */
	
/*
 * Success/Failure state in now stored in eax
 */
	__asm__ __volatile__("movl %%eax, %0"
			     :
			     : "m" (retval)
			    );

	outl( state, sci_en );	
}


static int nbsmi_smi_command(u16 function,const u8 *inputbuffer, u8 *outputbuffer)
{
	u32 retval;
	u32 sci_en;
	int count;
	
	for(count = 0; count < BUFFER_SIZE; count++) {
		outb( count + start_offset, RTC_PORT(2) );
		outb( *(inputbuffer + count), RTC_PORT(3) );
	}


/* 
 * We have to write 0xe4XX to smi_port
 * where XX is the SMI function code
 */	
	function = ( function & 0xff ) << 8;
	function |= 0xe4;
	

	switch (lpc_bridge->vendor) {
		case PCI_VENDOR_ID_INTEL:
/* 
 * We get the PMBASE offset ( bits 15:7 at 0x40 offset of PCI config space )
 * And we access offset 2c (GPE0_EN), save the state, disable all SCI
 * and restore the state in intel_do_smi_call function
 */
			pci_read_config_dword(lpc_bridge, INTEL_PMBASE, &sci_en);
			sci_en = sci_en & 0xff80; /* Keep bits 15:7 */
			sci_en += INTEL_GPE0_EN;  /* GPEO_EN offset */
			save_all_regs_flag();
			intel_do_smi_call(&retval,function,sci_en);
			restore_all_regs_flag();
			break;
		case PCI_VENDOR_ID_ATI:
			save_all_regs_flag();
			ati_do_smi_call(&retval,function);
			restore_all_regs_flag();
			break;
		default:
			BUG();
	}

	if(retval)
		printk(O_ERR "smi_command failed with error %i.\n", retval);

	for(count = 0; count < BUFFER_SIZE; count++) {
		outb( count + start_offset, RTC_PORT(2) );
		*(outputbuffer + count) = inb( RTC_PORT(3) );
	}
	
	return retval;
}


static int nbsmi_smi_read_command(const struct omnibook_operation *io_op, u8 *data)
{
	int retval;
	u8 *inputbuffer;
	u8 *outputbuffer;
	
	if(!lpc_bridge)
		return -ENODEV;

	if(mutex_lock_interruptible(&smi_lock))
		return -ERESTARTSYS;
	
	inputbuffer = kcalloc(BUFFER_SIZE,sizeof(u8),GFP_KERNEL);
	if(!inputbuffer) {
		retval = -ENOMEM;
		goto error1;
	}
	
	outputbuffer = kcalloc(BUFFER_SIZE,sizeof(u8),GFP_KERNEL);
	if(!inputbuffer) {
		retval = -ENOMEM;
		goto error2;
	}
	
	retval = nbsmi_smi_command( (u16) io_op->read_addr, inputbuffer, outputbuffer);
	if(!retval)
		goto out;
	
	*data = outputbuffer[0];

	if(io_op->read_mask)
		*data &= io_op->read_mask;

out:
	kfree(outputbuffer);
error2:
	kfree(inputbuffer);
error1:
	mutex_unlock(&smi_lock);
	return retval;
}

static int nbsmi_smi_write_command(const struct omnibook_operation *io_op, u8 data) 
{
	int retval;
	u8 *inputbuffer;
	u8 *outputbuffer;
	
	if(!lpc_bridge)
		return -ENODEV;

	if(mutex_lock_interruptible(&smi_lock))
		return -ERESTARTSYS;
	
	inputbuffer = kcalloc(BUFFER_SIZE,sizeof(u8),GFP_KERNEL);
	if(!inputbuffer) {
		retval = -ENOMEM;
		goto error1;
	}
	
	outputbuffer = kcalloc(BUFFER_SIZE,sizeof(u8),GFP_KERNEL);
	if(!inputbuffer) {
		retval = -ENOMEM;
		goto error2;
	}
	
	inputbuffer[0] = data;

	retval = nbsmi_smi_command( (u16) io_op->write_addr, inputbuffer, outputbuffer);
	
	kfree(outputbuffer);
error2:
	kfree(inputbuffer);
error1:
	mutex_unlock(&smi_lock);
	return retval;
}


/*
 * Read/Write to INDEX/DATA interface at port 0x300 (SMSC Mailbox registers)
 * Used by Hotkeys feature under already taken mutex.
 */
static void nbsmi_ec_read_command(u16 index, u16 *data)
{
	spin_lock_irq(&smi_spinlock);
	outw(index,EC_INDEX_PORT);
	*data = inw(EC_DATA_PORT);	
	spin_unlock_irq(&smi_spinlock);
}

static void nbsmi_ec_write_command(u16 index, u16 data)
{
	spin_lock_irq(&smi_spinlock);
	outw(index,EC_INDEX_PORT);
	outw(data,EC_DATA_PORT);
	spin_unlock_irq(&smi_spinlock);
}

/*
 * Try to init the backend
 * This function can be called blindly as it use a kref
 * to check if the init sequence was already done.
 */

static int omnibook_nbsmi_init(const struct omnibook_operation *io_op)
{
	int retval = 0;
	int i;
	u16 ec_data;
	u32 smi_port = 0;

/* ectypes other than TSM40 have no business with this backend */	
	if(!(omnibook_ectype & TSM40))
		return -ENODEV;

	if(already_failed) {
		dprintk("NbSmi backend init already failed, skipping.\n");
		return -ENODEV;
	}

	if(!refcount) {
		/* Fist use of the backend */
		mutex_lock(&smi_lock);
		dprintk("Try to init NbSmi\n");	
		refcount = kmalloc(sizeof(struct kref),GFP_KERNEL);
		if(!refcount) {
			retval= -ENOMEM;
			goto out;
		}

		kref_init(refcount);

		/* PCI probing: find the LPC Super I/O bridge PCI device */
		for (i = 0; !lpc_bridge && lpc_bridge_table[i].vendor; ++i)
			lpc_bridge = pci_get_device(lpc_bridge_table[i].vendor, lpc_bridge_table[i].device, NULL);

		if (!lpc_bridge) {
			printk(O_ERR "Fail to find a supported LPC I/O bridge, please report\n");
			retval = -ENODEV;
			goto error1;
		}
		
		if((retval = pci_enable_device(lpc_bridge))) {
			printk(O_ERR "Unable to enable PCI device.\n");
			goto error2;
		}
		
		switch (lpc_bridge->vendor) {
		case PCI_VENDOR_ID_INTEL:
			start_offset = INTEL_OFFSET;
			smi_port = INTEL_SMI_PORT;
			break;
		case PCI_VENDOR_ID_ATI:
			start_offset = ATI_OFFSET;
			smi_port = ATI_SMI_PORT;
			break;
		default:
			BUG();
		}
		
		if(!request_region(smi_port, 2 , OMNIBOOK_MODULE_NAME)) {
			printk(O_ERR "Request SMI I/O region error\n");
			retval = -ENODEV;
			goto error2;
		}

		if(!request_region(EC_INDEX_PORT, 2 , OMNIBOOK_MODULE_NAME)) {
			printk(O_ERR "Request EC I/O region error\n");
			retval = -ENODEV;
			goto error3;
		}


		/*
 		 * Try some heuristic tests to avoid enabling this interface on unsuported laptops:
 		 * See what a port 300h read index 8f gives. Guess there is nothing if read 0xffff
 		 */

		nbsmi_ec_read_command(SMI_FN_PRESSED, &ec_data);
		dprintk("NbSmi test probe read: %x\n",ec_data);
		if(ec_data == 0xffff) {
			printk(O_ERR "Probing at SMSC Mailbox registers failed, disabling NbSmi\n");
			retval = -ENODEV;
			goto error4;
		}
	
	
		dprintk("NbSmi init ok\n");
		goto out;
	} else {
		dprintk("NbSmi has already been initialized\n");
		kref_get(refcount);
		return 0;
	}
error4:
	release_region(EC_INDEX_PORT,2);
error3:
	release_region(smi_port,2);
error2:
	pci_dev_put(lpc_bridge);
	lpc_bridge = NULL;
error1:
	kfree(refcount);
	refcount = NULL;
	already_failed = 1;
out:
	mutex_unlock(&smi_lock);
	return retval;
}

static void nbsmi_free(struct kref *ref)
{
	u32 smi_port = 0;

	mutex_lock(&smi_lock);
	dprintk("NbSmi not used anymore: disposing\n");
	
	switch (lpc_bridge->vendor) {
	case PCI_VENDOR_ID_INTEL:
		smi_port = INTEL_SMI_PORT;
		break;
	case PCI_VENDOR_ID_ATI:
		smi_port = ATI_SMI_PORT;
		break;
	default:
		BUG();
	}
	
	pci_dev_put(lpc_bridge);
	release_region(smi_port,2);
	release_region(EC_INDEX_PORT,2);
	kfree(refcount);
	lpc_bridge = NULL;
	refcount = NULL;
	mutex_unlock(&smi_lock);
}

static void omnibook_nbsmi_exit(const struct omnibook_operation *io_op)
{
/* ectypes other than TSM40 have no business with this backend */
	BUG_ON(!(omnibook_ectype & TSM40));
	dprintk("Trying to dispose NbSmi\n");
	kref_put(refcount,nbsmi_free);
}

static int omnibook_nbsmi_get_wireless(const struct omnibook_operation *io_op, unsigned int *state)
{
	int retval = 0;
	struct omnibook_operation aerial_op;
	u8 data;
	
	aerial_op.read_addr = SMI_GET_KILL_SWITCH;

	if((retval = nbsmi_smi_read_command(&aerial_op, &data)))
		goto out;
	
	*state = data ? KILLSWITCH : 0; /* Make it 1 or 0 */

	aerial_op.read_addr = SMI_GET_AERIAL;

	if((retval = nbsmi_smi_read_command(&aerial_op, &data)))
		goto out;

	*state |= ( data & WLEX_MASK ) ? WIFI_EX : 0;
	*state |= ( data & WLAT_MASK ) ? WIFI_STA : 0;
	*state |= ( data & BTEX_MASK ) ? BT_EX : 0;
	*state |= ( data & BTAT_MASK ) ? BT_STA : 0;

out:
	return retval;
}


static int omnibook_nbsmi_set_wireless(const struct omnibook_operation *io_op, unsigned int state)
{
	int retval = 0;
	u8 data = 0;
	struct omnibook_operation aerial_op;

	aerial_op.write_addr = SMI_SET_AERIAL;
	
	data |= state & BT_STA;
	data |= ( state & WIFI_STA ) << 0x1 ;

	retval = nbsmi_smi_write_command(&aerial_op, data);
	
	return retval;
}

/*
 * Hotkeys handling
 */
static int omnibook_nbmsi_hotkeys_get(const struct omnibook_operation *io_op, unsigned int *state)
{
	int retval;
	u8 data = 0;
	struct omnibook_operation hotkeys_op;
	
	hotkeys_op.read_addr = SMI_GET_FN_INTERFACE;

	retval = nbsmi_smi_read_command(&hotkeys_op, &data);
	if(retval < 0)
		return retval;	

	*state = ( data & SMI_FN_KEYS_MASK ) ? HKEY_FN : 0;
	*state |= ( data & SMI_STICK_KEYS_MASK ) ? HKEY_STICK : 0;
	*state |= ( data & SMI_FN_TWICE_LOCK_MASK ) ? HKEY_TWICE_LOCK : 0;
	*state |= ( data & SMI_FN_DOCK_MASK ) ? HKEY_DOCK : 0;

	return HKEY_FN|HKEY_STICK|HKEY_TWICE_LOCK|HKEY_DOCK;
}

static int omnibook_nbmsi_hotkeys_set(const struct omnibook_operation *io_op, unsigned int state)
{
	int retval;
	u8 data = 0;
	struct omnibook_operation hotkeys_op;

	hotkeys_op.write_addr = SMI_SET_FN_INTERFACE;
	data |= ( state & HKEY_FN ) ? SMI_FN_KEYS_MASK : 0;
	data |= ( state & HKEY_STICK) ? SMI_STICK_KEYS_MASK : 0;
	data |= ( state & HKEY_TWICE_LOCK) ? SMI_FN_TWICE_LOCK_MASK : 0;
	data |= ( state & HKEY_DOCK) ? SMI_FN_DOCK_MASK : 0;

	retval = nbsmi_smi_write_command(&hotkeys_op, data);
	if(retval < 0)
		return retval;

	hotkeys_op.write_addr = SMI_SET_FN_F5_INTERFACE;
	data = !!( state & HKEY_FNF5); 

	retval = nbsmi_smi_write_command(&hotkeys_op, data);
	if(retval < 0)
		return retval;
	else
		return HKEY_FN|HKEY_STICK|HKEY_TWICE_LOCK|HKEY_DOCK|HKEY_FNF5;
}

static const unsigned int nbsmi_display_mode_list[] = {
	DISPLAY_LCD_ON,
	DISPLAY_LCD_ON|DISPLAY_CRT_ON,
	DISPLAY_CRT_ON,
	DISPLAY_LCD_ON|DISPLAY_TVO_ON,
	DISPLAY_TVO_ON,
};

static int  omnibook_nbmsi_display_get(const struct omnibook_operation *io_op, unsigned int *state)
{
	int retval = 0;
	u8 data;
	struct omnibook_operation display_op;

	display_op.read_addr = SMI_GET_DISPLAY_STATE;

	retval = nbsmi_smi_read_command(&display_op, &data);
	if(retval < 0)
		return retval;

	if( data > (ARRAY_SIZE(nbsmi_display_mode_list) - 1))
		return -EIO;	
	
	*state = nbsmi_display_mode_list[data];

	return DISPLAY_LCD_ON|DISPLAY_CRT_ON|DISPLAY_TVO_ON;
}

static int omnibook_nbmsi_display_set(const struct omnibook_operation *io_op, unsigned int state)
{
	int retval = 0;
	int i;
	u8 matched = 0;
	struct omnibook_operation display_op;

	display_op.write_addr = SMI_SET_DISPLAY_STATE;

	for(i = 0; i < ARRAY_SIZE(nbsmi_display_mode_list); i++) {
		if(nbsmi_display_mode_list[i] == state) {
			matched = i;
			break;
		}	
	}
	if(!matched) {
		printk("Display mode %x is unsupported.\n", state);
		return -EINVAL;
	}
	
	retval = nbsmi_smi_write_command(&display_op, matched);
	if(retval < 0)
		return retval;

	return DISPLAY_LCD_ON|DISPLAY_CRT_ON|DISPLAY_TVO_ON;
}



struct omnibook_backend nbsmi_backend = {
	.name = "nbsmi",
	.init = omnibook_nbsmi_init,
	.exit = omnibook_nbsmi_exit,
	.byte_read = nbsmi_smi_read_command,
	.byte_write = nbsmi_smi_write_command,
	.aerial_get = omnibook_nbsmi_get_wireless,
	.aerial_set = omnibook_nbsmi_set_wireless,
	.hotkeys_get = omnibook_nbmsi_hotkeys_get,
	.hotkeys_set = omnibook_nbmsi_hotkeys_set,
	.display_get = omnibook_nbmsi_display_get,
	.display_set = omnibook_nbmsi_display_set,
};
