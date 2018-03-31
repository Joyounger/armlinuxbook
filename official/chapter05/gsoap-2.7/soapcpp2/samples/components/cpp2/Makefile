CC=gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=-Wall
CIFLAGS=-I.. -I../.. -I../../..
CMFLAGS=
CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)
main:		main.cpp envC.o stdsoap2.o quoteServiceProxy.o rateServiceProxy.o calcServiceService.o
		$(CPP) $(CFLAGS) -o main main.cpp envC.o stdsoap2.o quoteServiceProxy.o rateServiceProxy.o calcServiceService.o quoteC.cpp rateC.cpp calcC.cpp
stdsoap2.o:	stdsoap2.cpp
		$(CPP) $(CFLAGS) -DWITH_NONAMESPACES -c stdsoap2.cpp
stdsoap2.cpp:
		cp -f ../../../stdsoap2.cpp .
envC.o:		env.h header.h fault.h
		soapcpp2 -CS -penv env.h
		$(CPP) $(CFLAGS) -c envC.cpp
quoteServiceProxy.cpp:	quote.h
		soapcpp2 -C -ni quote.h
quoteServiceProxy.o:	quoteServiceProxy.cpp
		$(CPP) $(CFLAGS) -c quoteServiceProxy.cpp
rateServiceProxy.cpp:	rate.h
		soapcpp2 -C -ni rate.h
rateServiceProxy.o:	rateServiceProxy.cpp
		$(CPP) $(CFLAGS) -c rateServiceProxy.cpp
calcServiceService.cpp:	calc.h
		soapcpp2 -S -ni calc.h
calcServiceService.o:	calcServiceService.cpp
		$(CPP) $(CFLAGS) -c calcServiceService.cpp
clean:		
		rm -f *.o *.nsmap *H.h *Stub.h *C.cpp *C.c *Client.cpp *Client.c *ClientLib.c *ClientLib.cpp *Server.cpp *Server.c *ServerLib.c *ServerLib.cpp *ServiceObject.h *ServiceProxy.h
distclean:		
		rm -f main *.o *.wsdl *.xsd *.xml *.nsmap *H.h *Stub.h *C.cpp *C.c *Client.cpp *Client.c *ClientLib.c *ClientLib.cpp *Server.cpp *Server.c *ServerLib.c *ServerLib.cpp *ServiceObject.h *ServiceProxy.h *ServiceService.h *ServiceProxy.cpp *ServiceService.cpp
