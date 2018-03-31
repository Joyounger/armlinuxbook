GSOAP=../../bin/soapcpp2
GWSDL=../../bin/wsdl2h
SOAPH=../../stdsoap2.h
SOAPC=../../stdsoap2.c
SOAPCPP=../../stdsoap2.cpp
CC=gcc
CPP=g++
LIBS=-lcrypto -lssl
COFLAGS=-O2
CWFLAGS=-Wall
CIFLAGS=-I. -I../.. -I../../plugin
CMFLAGS=-DWITH_DOM -DWITH_OPENSSL
CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)
all:		wsademo
wsademo:	wsademo.h wsademo.c soapC.c $(SOAPH) $(SOAPC) wsaapi.o
		$(CC) $(CFLAGS) -o wsademo wsademo.c soapC.c soapClient.c soapServer.c ../../dom.c $(SOAPC) wsaapi.o $(LIBS)
soapC.c:	wsademo.h
		$(GSOAP) -c -I../../import wsademo.h
wsaapi.o:	soapC.c ../../plugin/wsaapi.h ../../plugin/wsaapi.c
		$(CC) $(CFLAGS) -c ../../plugin/wsaapi.c
clean:
		rm -f *.o soapH.h soapStub.h soapC.cpp soapC.c soapClient.cpp soapClient.c soapServer.cpp soapServer.c soap*Proxy.h soap*Object.h soapClientLib.c soapServerLib.c
distclean:
		rm -f *.o *.wsdl *.xml *.nsmap *.log soapH.h soapStub.h soapC.cpp soapC.c soapClient.cpp soapClient.c soapServer.cpp soapServer.c soapClientLib.c soapServerLib.c soap*Proxy.h soap*Object.h wsademo
