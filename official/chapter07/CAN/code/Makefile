#TOPDIR = ../
#include $(TOPDIR)/Rules.mak

CC=armv4l-unknown-linux-gcc
EXTRA_LIBS += -lpthread

EXEC= cantest

OBJS= cantest.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)  $(EXTRA_LIBS)  

install:
	$(EXP_INSTALL) $(EXEC) $(INSTALL_DIR)

clean:
	rm -f *.o a.out cantest *.gdb
