CC=armv4l-unknown-linux-gcc
EXEC=hello
OBJS=hello.o
CFLAGS+=
LDFLAGS+=-static

all:$(EXEC)
$(EXEC):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

clean:
	-rm -f $(EXEC) *.elf *.o *.gdb
