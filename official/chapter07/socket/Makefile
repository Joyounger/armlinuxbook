CC1=armv4l-unknown-linux-gcc
CC2=gcc
EXEC1=skt_ser
EXEC2=skt_cli
CFLAGS+=
 
all:$(EXEC1) $(EXEC2)
$(EXEC1):skt_ser.c
        $(CC1) -o $@ skt_ser.c
$(EXEC2):skt_cli.c
        $(CC2) -o $@ skt_cli.c
clean:
-rm -f $(EXEC1) $(EXEC2) *.elf *.o *.gdb
