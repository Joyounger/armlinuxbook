CC = /opt/crossdev/gcc-3.4.2-glibc-2.2.5/arm-linux/bin/arm-linux-gcc


CFLAGS = -Wall -I/home/lipeng/Linux-2.6.12/include
DSP_BIN = dspctl

all: $(DSP_BIN)

clean:
	-rm $(DSP_BIN) *.o

dspctl: dspctl.o dspload.o
dspctl.o: dspctl.c dspctl.h
dspload.o: dspload.c coff-c55x.h dspctl.h
