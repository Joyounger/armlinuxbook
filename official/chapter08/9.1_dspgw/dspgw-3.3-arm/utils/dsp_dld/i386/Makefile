#
# Set your enviroment here
#
LINUX = /usr/src/linux-2.6.12-omap1-dspgw3.2
CC = gcc
YACC = bison
LEX = flex
LFLAGS = -lfl
CFLAGS = -g -Wall -DDSP_EMULATION -I${LINUX}/include
#CFLAGS += -DDEBUG_ALL
#CFLAGS += -DDEBUG_SECTION -DDEBUG_RELOCATE
#CFLAGS += -DDEBUG_MEMMGR -DDEBUG_SECTION
#CFLAGS += -DDEBUG_SYMBOL -DDEBUG_LOAD -DDEBUG_RELOCATE
##CFLAGS += -DDEBUG_MEMMGR -DDEBUG_SECTION
#CFLAGS += -DDEBUG_SECTION -DDEBUG_CINIT
#CFLAGS += -DDEBUG_MALLOC

#
# use global list (memmgr, symbol, section)
# This is safe from changing kernel.out or global linker fommand file
# but consumes more memory.
#
#CFLAGS += -DSTICKY_LIST

#
# fork child process for tadd/tdel/tkill
#
CFLAGS += -DUSE_FORK

OBJS =	dsp_dld.o\
	dld_malloc.o\
	dld_daemon.o\
	dld_server.o\
	dld_taskent.o\
	dld_cmd.o\
	dld_coff.o\
	dld_section.o\
	dld_symbol.o\
	dld_memmgr.o\
	y.tab.o

all: dsp_dld dld_client

dsp_dld: $(OBJS)

dsp_dld.o: dsp_dld.c coff-c55x.h dsp_dld.h dld_daemon.h dld_server.h dld_taskent.h dld_memmgr.h dld_cmd.h dld_coff.h dld_section.h dld_symbol.h
dld_malloc.o: dld_malloc.c dsp_dld.h dld_daemon.h
dld_daemon.o: dld_daemon.c
dld_server.o: dld_server.c dsp_dld.h dld_daemon.h
dld_cmd.o: dld_cmd.c dsp_dld.h dld_malloc.h dld_daemon.h
dld_taskent.o: dld_taskent.c dsp_dld.h dld_malloc.h dld_daemon.h
dld_coff.o: dld_coff.c coff-c55x.h dsp_dld.h dld_malloc.h dld_daemon.h dld_section.h dld_symbol.h
dld_section.o: dld_section.c coff-c55x.h dsp_dld.h dld_malloc.h dld_daemon.h dld_coff.h
dld_symbol.o: dld_symbol.c coff-c55x.h dsp_dld.h dld_malloc.h dld_daemon.h dld_coff.h
dld_memmgr.o: dld_memmgr.c dsp_dld.h dld_malloc.h dld_daemon.h dld_cmd.h
y.tab.o: y.tab.c dsp_dld.h dld_malloc.h dld_daemon.h dld_cmd.h

dld_client.o: dld_client.c dsp_dld.h

lex.yy.c : dld_cmd_spec.l
	$(LEX) dld_cmd_spec.l

y.tab.c : dld_cmd_spec.y lex.yy.c
	$(YACC) -y dld_cmd_spec.y

clean:
	-rm dsp_dld $(OBJS) y.tab.c lex.yy.c dld_client dld_client.o
