#CC = /usr/local/arm/bin/arm-linux-gcc
CC =  /opt/crossdev/gcc-3.4.2-glibc-2.2.5/arm-linux/bin/arm-linux-gcc

CFLAGS = -I/home/lipeng/Linux-2.6.12/dspgw-3.3-arm/ -Wall
BIN = demo_console demo_fb-inno demo_fbtest

all: $(BIN)

clean:
	-rm $(BIN)

demo_fb-inno: demo_fb.c
	$(CC) $(CFLAGS) -DINNOVATOR $< -o $@
