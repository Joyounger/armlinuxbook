
include ../config.mak

LIBNAME = libmenu.a

SRCS= menu.c vf_menu.c menu_cmdlist.c menu_pt.c menu_list.c menu_filesel.c menu_txt.c menu_console.c menu_param.c
ifeq ($(DVBIN),yes)
SRCS += menu_dvbin.c
EXTRA_INC += $(DVB_INC)
endif
OBJS=$(SRCS:.c=.o)

CFLAGS  = $(OPTFLAGS) -I. -I.. -I../libmpcodecs $(EXTRA_INC) -Wall

.SUFFIXES: .c .o


.c.o:
	$(CC) -c $(CFLAGS) $(FREETYPE_INC) -o $@ $<

$(LIBNAME):     $(OBJS)
	$(AR) r $(LIBNAME) $(OBJS)
	$(RANLIB) $(LIBNAME)

all:    $(LIBNAME)

clean:
	rm -f *.o *.a *~

distclean:
	rm -f Makefile.bak *.o *.a *~ .depend

dep:    depend

depend:
	$(CC) -MM $(CFLAGS) $(FREETYPE_INC) $(SRCS) 1>.depend

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
