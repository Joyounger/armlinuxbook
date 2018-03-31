CC = gcc
LFLAGS = -lfl
CFLAGS = -g -Wall

LIBOBJS = coffobj.o section.o symbol.o

all: cofflib.a

clean:
	-rm *.o cofflib.a

cofflib.a: $(LIBOBJS)
	ar r cofflib.a $(LIBOBJS)

coffobj.o: coffobj.c coff-c55x.h cofflib.h section.h symbol.h
section.o: section.c list.h coff-c55x.h cofflib.h coffobj.h symbol.h section.h
symbol.o: symbol.c list.h coff-c55x.h cofflib.h coffobj.h
