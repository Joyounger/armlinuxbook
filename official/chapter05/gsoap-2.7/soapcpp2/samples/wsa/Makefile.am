####### This is the input file for automake, which will generate Makefile.in ##########
## not a GNU package. You can remove this line, if
## you have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

noinst_PROGRAMS=wsademo
SOAPHEADER=wsademo.h

include ../Makefile.defines
SOAP_NS=wsademo.nsmap wsademo.wsdl
include ../Makefile.c_rules

wsademo_SOURCES=wsademo.c $(SOAPHEADER) $(SOAP_C_SRC)
wsademo_LDADD=$(SOAP_C_LIB) $(SAMPLE_EXTRA_LIBS)


