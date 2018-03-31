CC=armv4l-unknown-linux-gcc
#CC=gcc
OBJS2=exampthread.o
EXEC2=exampthread

all:$(EXEC2)
$(EXEC2):$(OBJS2)
	$(CC) -o $@ $(OBJS2) -lpthread

clean:
	rm -f $(EXEC2) $(OBJS2)
