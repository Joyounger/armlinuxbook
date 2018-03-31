#
# Makefile for demo_console.o
#

CCS_DIR = /opt/ti_dsptools
CC = $(CCS_DIR)/cgtools-c5500-v2.56/bin/cl55
LD = $(CC)
TOKLI_DIR = ../../tokliBIOS
TINKERNEL = ../../tinkernel/tinkernel.out

#
# compiler options
#
INCD =	-i$(TOKLI_DIR) \
	-i$(CCS_DIR)/cgtools-c5500-v2.56/include \
	-i$(CCS_DIR)/bios_5_03/ti/bios/include \
	-i$(CCS_DIR)/ccs221_content/c5500/xdais/include \
	-i$(CCS_DIR)/ccs221_content/c5500/xdais/src/api

CFLAGS = -q -ml

#
# linker options
#
LIBD =	-i$(CCS_DIR)/cgtools-c5500-v2.56/lib \
	-i$(CCS_DIR)/bios_5_03/ti/bios/lib \
	-i$(CCS_DIR)/ccs221_content/c5500/rtdx/lib

LDFLAGS = -z -x -r -cr

LIBS =	-llnknone.a55L \
	-ldrivers.a55L \
	-lsioboth.a55L \
	-lbios1510.a55L \
	-lbios.a55L \
	-lrtdxx.lib \
	-lrts55x.lib

all: demo_console.o

clean:
	-rm *.obj demo_console.o

.c.o:
	$(CC) $(CFLAGS) $(INCD) $<
	$(LD) $(LDFLAGS) $*.obj dummy_kernel.obj dummy.cmd $(LIBD) $(LIBS) -o$@
	coff_unresolve -s .tinkernel $@

demo_console.o: demo_console.c dummy_kernel.obj

dummy_kernel.obj: $(TINKERNEL)
	gen_dummy_kernel $< -o $@

$(TINKERNEL):
	@echo
	@echo '************************************************'
	@echo '$(TINKERNEL) not found.'
	@echo '************************************************'
	@echo
	@exit 1
