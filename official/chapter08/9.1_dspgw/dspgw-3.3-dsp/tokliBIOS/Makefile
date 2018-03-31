#
# Makefile for tokliBIOS.lib, for Linux DSP Tools
#

TOOLS_DIR = /opt/ti_dsptools
CC = $(TOOLS_DIR)/cgtools-c5500-v2.56/bin/cl55
AR = $(TOOLS_DIR)/cgtools-c5500-v2.56/bin/ar55
AS = $(CC)
LD = $(CC)
TCONF = $(TOOLS_DIR)/bios_5_03/tconf/tconf

INCD =	-i$(TOOLS_DIR)/cgtools-c5500-v2.56/include \
	-i$(TOOLS_DIR)/bios_5_03/ti/bios/include

CFLAGS = -q -ml

LIBD =	-i$(TOOLS_DIR)/cgtools-c5500-v2.56/lib \
	-i$(TOOLS_DIR)/bios_5_03/ti/bios/lib \
	-i$(TOOLS_DIR)/ccs221_content/c5500/rtdx/lib

TRGT = tokliBIOS.lib
LIB_OBJS =	tokliBIOScfg.obj \
		tokliBIOScfg_c.obj \
		tokliBIOS.obj \
		supertask.obj \
		usertask.obj \
		idle.obj \
		suspend.obj \
		wdt.obj \
		timer.obj \
		dspuart.obj

.SUFFIXES: .obj .s55

all: $(TRGT)

clean:
	-rm $(TRGT) *.obj \
	    tokliBIOScfg.s55 \
	    tokliBIOScfg.h55 \
	    tokliBIOScfg_c.c \
	    tokliBIOScfg.h \
	    tokliBIOS.cdb \
	    tokliBIOScfg.cmd

tokliBIOS.lib: $(LIB_OBJS)
	$(AR) -r $@ $(LIB_OBJS)

tokliBIOScfg.s55: tokliBIOScfg.tcf
	$(TCONF) -Dconfig.importPath=$(TOOLS_DIR)/bios_5_03 $<

tokliBIOScfg.obj: tokliBIOScfg.s55
tokliBIOScfg_c.obj: tokliBIOScfg_c.c
tokliBIOS.obj: tokliBIOS.c tokliBIOS.h tokliBIOSlib.h tokliBIOScfg.h omap1510.h mailbox.h
supertask.obj: supertask.c tokliBIOS.h tokliBIOSlib.h tokliBIOScfg.h omap1510.h mailbox.h
usertask.obj: usertask.c tokliBIOS.h tokliBIOSlib.h tokliBIOScfg.h omap1510.h mailbox.h
idle.obj: idle.c omap1510.h timer.h
suspend.obj: suspend.s
wdt.obj: wdt.c tokliBIOSlib.h omap1510.h mailbox.h timer.h
timer.obj: timer.c tokliBIOSlib.h omap1510.h mailbox.h queue.h timer.h
dspuart.obj: dsp5910.h

.c.obj:
	$(CC) $(CFLAGS) $(INCD) $<

.s.obj:
	$(AS) $(CFLAGS) $(INCD) $<

.s55.obj:
	$(AS) $(CFLAGS) $(INCD) $<
