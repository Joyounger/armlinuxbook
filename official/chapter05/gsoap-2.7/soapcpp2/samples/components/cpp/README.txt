
This example illustrates the use of C++ namespaces to combine two separately
compiled clients and server into one executable. Each C++ namespace
encapsulates a Web service proxy or server object.

The proxies and server object share SOAP header and fault definitions declared
outside their C++ namespaces, i.e. in the standard namespace. The
SOAP_ENV__Header and SOAP_ENV__Fault structures must include all members needed
to support the client proxies and server. Therefore, the env.h includes
header.h and fault.h and is separately parsed with soapcpp2 to generate code
for SOAP Header and Fault handling.

To generate non-client-server header and fault handlers:
$ soapcpp2 -CS -penv env.h

The stock quote client declared in 'quote' namespace:
$ soapcpp2 -C -n quote.h

The exchange rate client declared in 'rate' namespace:
$ soapcpp2 -C -n rate.h

The calculator server declared in the 'calc' namespace:
$ soapcpp2 -S -n calc.h

See main.cpp for example code.
