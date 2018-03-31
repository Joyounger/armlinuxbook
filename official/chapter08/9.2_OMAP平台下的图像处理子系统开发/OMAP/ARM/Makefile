CC = /usr/local/arm/bin/arm-linux-gcc

CFLAGS = -I/usr/src/linux-2.6.12-omap1-dspgw3.3/include -Wall
DSP_BIN = \
	graph\

all: $(DSP_BIN)

clean:
	-rm $(DSP_BIN)

#hellotimer: hellotimer.c
graph: graph.c
