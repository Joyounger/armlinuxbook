CC = gcc
LFLAGS = -lfl
CFLAGS = -g -Wall -Ilib

SUBDIRS = lib
TRGT = coff_unresolve gen_dummy_kernel

all:
	-for d in $(SUBDIRS); do $(MAKE) -C $$d $@; done
	-$(MAKE) $(TRGT)

clean:
	-rm $(TRGT) *.o
	-for d in $(SUBDIRS); do $(MAKE) -C $$d $@; done

coff_unresolve: coff_unresolve.o lib/cofflib.a
gen_dummy_kernel: gen_dummy_kernel.o lib/cofflib.a
