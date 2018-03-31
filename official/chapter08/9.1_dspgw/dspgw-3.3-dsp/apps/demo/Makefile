#
# Makefile for DSP demo applications, for Linux DSP Tools
#

TOOLS_DIR = /opt/ti_dsptools
CC = $(TOOLS_DIR)/cgtools-c5500-v2.56/bin/cl55
AS = $(CC)
LD = $(CC)
TOKLI_DIR = ../../tokliBIOS

INCD =	-i$(TOKLI_DIR) \
	-i$(TOOLS_DIR)/cgtools-c5500-v2.56/include \
	-i$(TOOLS_DIR)/bios_5_03/ti/bios/include \
	-i$(TOOLS_DIR)/ccs221_content/c5500/xdais/include \
	-i$(TOOLS_DIR)/ccs221_content/c5500/xdais/src/api

CFLAGS = -q -ml

LIBD =	-i$(TOKLI_DIR) \
	-i$(TOOLS_DIR)/cgtools-c5500-v2.56/lib \
	-i$(TOOLS_DIR)/bios_5_03/ti/bios/lib \
	-i$(TOOLS_DIR)/ccs221_content/c5500/rtdx/lib

LDFLAGS = -z -q -cr -x

LIBS = -ltokliBIOS.lib

TRGT = demo_console.out demo_fb.out
CONS_OBJS = sysinfo_cons.obj demo_console.obj
FB_OBJS = sysinfo_fb.obj demo_fb.obj
CMDS = demo.cmd

.SUFFIXES: .obj

.c.obj:
	$(CC) $(CFLAGS) $(INCD) $<

all: $(TRGT)

clean:
	-rm $(TRGT) $(CONS_OBJS) $(FB_OBJS)

demo_console.out: $(CONS_OBJS) $(CMDS)
	$(LD) $(LDFLAGS) $(LIBD) $(LIBS) $(CONS_OBJS) $(CMDS) -o$@
demo_fb.obj: demo_fb.c char-bitmap.h
demo_fb.out: $(FB_OBJS) $(CMDS)
	$(LD) $(LDFLAGS) $(LIBD) $(LIBS) $(FB_OBJS) $(CMDS) -o$@
