#
# Makefile -- makefile for the HP OmniBook support module
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# Written by Soós Péter <sp@osb.hu>, 2002-2004
#

MODULE_NAME	= omnibook

ifeq ($(KERNELRELEASE),)

DESTDIR	= 
MODDIR	= $(DESTDIR)/lib/modules
KVERS	= $(shell uname -r)
KVER	= $(KVERS)
VMODDIR = $(MODDIR)/$(KVER)
INSTDIR	= extra
#KSRC	= /usr/src/linux
KSRC	= $(VMODDIR)/build
KMODDIR	= $(KSRC)/drivers/misc/omnibook
KINCDIR	= $(KSRC)/include/linux
KDOCDIR	= $(KSRC)/Documentation/omnibook
PWD	= $(shell pwd)
TODAY	= $(shell date +%Y%m%d)
KERNEL	= $(shell echo $(KVER) | cut -d . -f 1-2)

DEPMOD	= /sbin/depmod -a
RMMOD	= /sbin/modprobe -r
INSMOD	= /sbin/modprobe
INSTALL	= install -m 644
MKDIR	= mkdir -p
RM	= rm -f
FIND	= find
endif

DEBUG	= # -D OMNIBOOK_DEBUG  -g -O0

EXTRA_CFLAGS += -D OMNIBOOK_STANDALONE $(DEBUG)
EXTRA_LDFLAGS += $(src)/sections.lds


OBJS	= ac.o battery.o blank.o display.o dock.o \
	  ec.o fan.o fan_policy.o init.o lcd.o hotkeys.o \
	  temperature.o touchpad.o dump.o info.o \
	  apmemu.o muteled.o compal.o


# All extra flags delt with automagically

obj-m         += $(MODULE_NAME).o
omnibook-objs := $(OBJS)

all:		 $(MODULE_NAME).ko

clean:
		make -C $(KSRC) M=$(PWD) clean
		$(RM) *~ "#*#" .swp
		$(RM) -r debian/omnibook-source *-stamp
		$(RM) -r Module.symvers Modules.symvers
		(cd misc/obtest; $(RM) obtest *.o)


install:	all
		# Removing module from locations used by previous versions
		$(RM) $(VMODDIR)/kernel/drivers/char/$(MODULE_NAME).ko
		$(RM) $(VMODDIR)/kernel/drivers/misc/$(MODULE_NAME).ko
		make INSTALL_MOD_PATH=$(DESTDIR) INSTALL_MOD_DIR=$(INSTDIR) -C $(KSRC) M=$(PWD) modules_install

unload:
		$(RMMOD) $(MODULE_NAME) || :

load:		install unload
		$(DEPMOD)
		$(INSMOD) $(MODULE_NAME)

uninstall:	unload
		$(FIND) $(VMODDIR) -name "$(MODULE_NAME).ko" -exec $(RM) {} \;
		$(DEPMOD)

uninstall-all:	unload
		$(FIND) $(MODDIR) -name "$(MODULE_NAME).ko" -exec $(RM) {} \;
		$(DEPMOD)

$(MODULE_NAME).ko:
		$(MAKE) -C $(KSRC) SUBDIRS=$(PWD) modules

kinstall:
		$(RM) -r $(KMODDIR)
		$(RM) $(KINCDIR)/omnibook.h
		$(MKDIR) $(KMODDIR)
		$(INSTALL) *.c sections.lds $(KMODDIR)
		$(INSTALL) apmemu.h compat.h ec.h laptop.h $(KMODDIR)
		$(INSTALL) omnibook.h $(KINCDIR)
		$(MKDIR) $(KDOCDIR)
		$(INSTALL) doc/README doc/README-OneTouch $(KDOCDIR)
		
kpatch:		kinstall
		(cd $(KSRC); patch -p1 < $(PWD)/misc/omnibook-integration.patch)

version:	
		sed -i "s|^\(#define OMNIBOOK_MODULE_VERSION.*\)\".*\"|\1\"2.$(TODAY)\"|" omnibook.h
		sed -i "s|^\(2\.\)X\{8\}|\1$(TODAY)|" doc/ChangeLog

release:	clean version
		mkdir -p ../$(MODULE_NAME)-2.$(TODAY)
		cp -a *.h *.c *.lds Makefile doc misc ../$(MODULE_NAME)-2.$(TODAY)
		rm -f ../$(MODULE_NAME)-2.$(TODAY).tar ../$(MODULE_NAME)-2.$(TODAY).tar.gz
		(cd ..; tar cvf $(MODULE_NAME)-2.$(TODAY).tar $(MODULE_NAME)-2.$(TODAY); gzip -9 $(MODULE_NAME)-2.$(TODAY).tar)

# End of file
