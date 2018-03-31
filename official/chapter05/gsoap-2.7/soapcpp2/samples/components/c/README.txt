
This example illustrates the use of 'static' definitions to combine two
separately compiled clients and server into one executable. The xyzClientLib.c
and xyzServerLib.c codes hide the serializers and auxiliary routines by
declaring these static, thus making them only locally accessible by the call
interface. The xyzClientLib.c and xyzServerLib.c combine xyzC.c and xyzClient.c
(or xyzServer.c) and therefore should NOT be linked again with xyzC.c,
xyzClient.c, and xyzServer.c

The clients and server share SOAP header and fault definitions declared in
header.h and fault.h. The SOAP_ENV__Header and SOAP_ENV__Fault structures must
include all members needed to support the clients and server. The env.h header,
which includes header.h and fault.h, is separately parsed with soapcpp2 to
generate code for SOAP Header and Fault handling.

To separate the generated files, use the soapcpp2 -n and -p options as shown
below.

To generate non-client-server header and fault handlers:
$ soapcpp2 -c -CS -penv env.h

The stock quote client:
$ soapcpp2 -c -C -n -pquote quote.h

The exchange rate client:
$ soapcpp2 -c -C -n -prate rate.h

The calculator server:
$ soapcpp2 -c -S -n -pcalc calc.h

See main.c for example code.
