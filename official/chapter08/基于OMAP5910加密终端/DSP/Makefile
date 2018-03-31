#
# Makefile for ex_task.out, for Linux DSP Tools
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

TRGT = ex_task.out
OBJS =	sysinfo.obj \
	rsa_key.obj \
	rsa_Enc.obj \
	rsa_Dec.obj \
	rc4.obj 

CMDS = ex_task.cmd

.SUFFIXES: .obj

.c.obj:
	$(CC) $(CFLAGS) $(INCD) $<

all: $(TRGT)

clean:
	-rm $(TRGT) $(OBJS)

ex_task.out: $(OBJS) $(CMDS) $(TOKLI_DIR)/tokliBIOS.lib
	$(LD) $(LDFLAGS) $(LIBD) $(LIBS) $(OBJS) $(CMDS) -o$@
