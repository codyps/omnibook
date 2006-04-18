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
KVER	= $(shell uname -r)
VMODDIR = $(MODDIR)/$(KVER)
INSTDIR	= $(VMODDIR)/kernel/drivers/misc/omnibook
#KSRC	= /usr/src/linux
KSRC	= $(VMODDIR)/build
KMODDIR	= $(KSRC)/drivers/misc/omnibook
KINCDIR	= $(KSRC)/include/linux
KDOCDIR	= $(KSRC)/Documentation/omnibook
BDIR	= $(shell pwd)
TODAY	= $(shell date +%Y%m%d)
KERNEL	= $(shell uname -r | cut -c 1-3)

DEPMOD	= depmod -a
RMMOD	= modprobe -r
INSMOD	= modprobe
INSTALL	= install -m 644
MKDIR	= mkdir -p
RM	= rm -f
FIND	= find
endif

DEBUG	=  # -D OMNIBOOK_DEBUG  -g -O0
# Used by 2.6 stuff, so let's be consistant.
EXTRA_CFLAGS += -D OMNIBOOK_STANDALONE $(DEBUG)

OBJS	= ac.o battery.o blank.o display.o dock.o \
	  ec.o fan.o fan_policy.o init.o lcd.o onetouch.o \
	  temperature.o touchpad.o dump.o info.o watch.o \
	  apmemu.o

# All extra flags delt with automagically
obj-m         += $(MODULE_NAME).o
omnibook-objs := $(OBJS)

all:		 $(MODULE_NAME).ko

clean:
		$(RM) .*.cmd *.map *.mod.c *.o *.ko *~ "#*#"
		$(RM) -r .tmp_versions
		$(RM) -r debian/omnibook-source *-stamp
		$(RM) -r Modules.symvers
		(cd misc/obtest; $(RM) obtest *.o)

install:	all
		# Removing module from old location
		$(RM) $(MODDIR)/char/$(MODULE_NAME).ko
		$(RM) $(MODDIR)/misc/$(MODULE_NAME).ko
		$(MKDIR) $(INSTDIR)
		$(INSTALL) $(MODULE_NAME).ko $(INSTDIR)
		$(DEPMOD)

unload:
		$(RMMOD) $(MODULE_NAME) || :

load:		install unload
		$(INSMOD) $(MODULE_NAME)

uninstall:	unload
		$(FIND) $(VMODDIR) -name "$(MODULE_NAME).ko" -exec $(RM) {} \;
		$(DEPMOD)

uninstall-all:	unload
		$(FIND) $(MODDIR) -name "$(MODULE_NAME).ko" -exec $(RM) {} \;
		$(DEPMOD)

$(MODULE_NAME).ko:
		PWD=$(shell pwd)
		$(MAKE) -C $(KSRC) SUBDIRS=$(PWD) modules

kinstall:
		$(RM) -r $(KMODDIR)
		$(RM) $(KINCDIR)/omnibook.h
		$(MKDIR) $(KMODDIR)
		$(INSTALL) *.c $(KMODDIR)
		$(INSTALL) apmemu.h ec.h init.h $(KMODDIR)
		$(INSTALL) omnibook.h $(KINCDIR)
		$(MKDIR) $(KDOCDIR)
		$(INSTALL) doc/README doc/README-OneTouch $(KDOCDIR)
		
kpatch:		kinstall
		(cd $(KSRC); patch -p1 < $(BDIR)/misc/omnibook-integration.patch)

deb:		clean
		dch -v $(TODAY)
		fakeroot dpkg-buildpackage
		

release:	clean
		mkdir -p ../$(MODULE_NAME)-$(TODAY)
		cp -a *.h *.c Makefile debian doc misc ../$(MODULE_NAME)-$(TODAY)
		sed -i "s|^\(#define OMNIBOOK_MODULE_VERSION.*\)\".*\"|\1\"$(TODAY)\"|" ../$(MODULE_NAME)-$(TODAY)/omnibook.h
		sed -i "s|^\(omnibook \)(current.*)|\1($(TODAY)-0)|" ../$(MODULE_NAME)-$(TODAY)/debian/changelog
		rm -f ../$(MODULE_NAME)-$(TODAY).tar ../$(MODULE_NAME)-$(TODAY).tar.gz
		(cd ..; tar cvf $(MODULE_NAME)-$(TODAY).tar $(MODULE_NAME)-$(TODAY); gzip -9 $(MODULE_NAME)-$(TODAY).tar)

current:	clean
		rm -f ../$(MODULE_NAME)-current.tar ../$(MODULE_NAME)-current.tar.gz
		(cd ..; tar cvf $(MODULE_NAME)-current.tar $(MODULE_NAME)-current; gzip -9 $(MODULE_NAME)-current.tar)

# End of file
