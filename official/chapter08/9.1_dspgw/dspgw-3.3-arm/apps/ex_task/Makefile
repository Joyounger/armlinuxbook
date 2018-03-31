CC = /usr/local/arm/bin/arm-linux-gcc

CFLAGS = -I/usr/src/linux-2.6.12-omap1-dspgw3.3/include -Wall
DSP_BIN = \
	hellotimer \
	test2 \
	test3 \
	test4 \
	test5 \
	test6 \
	test7 \
	test8 \
	endiantest \


all: $(DSP_BIN)

clean:
	-rm $(DSP_BIN)

hellotimer: hellotimer.c
test2: test2.c
test3: test3.c
test4: test4.c
test5: test5.c
test6: test6.c
test7: test7.c
test8: test8.c
endiantest: endiantest.c
