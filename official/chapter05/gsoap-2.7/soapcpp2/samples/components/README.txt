
How to combine multiple clients and servers into one executable?

1. Run wsdl2h once on all WSDLs together.
   The wsdl2h tool can import multiple WSDL files at once to combine multiple
   clients and service into one executable.

2. Run wsdl2h on each WSDL and use soapcpp2 compiler options.
   If there is a need to build clients and services from multiple gSOAP header
   files, then an alternative approach is required demonstrated by the material
   included in this part of the gSOAP package.

The advantages of #1 is that you end up with smaller code, because redundant
definitions are eliminated.

The advantage of #2 is a more modular setup, thus supporting dynamic linking of 
separately compiled service modules. However, the build process requires
additional steps depending on C or C++. The C++ project build process is
simpler, because C++ namespaces can be used to separate the definitions as is
shown in the 'cpp' and 'cpp2' directories.

The C and C++ examples in this directory illustrate how multiple clients and
services can be compiled and linked into one executable from multiple gSOAP
header files. The C examples accomplish this by merging statically declared
serializers. The C++ examples accomplish this by using C++ namespaces to
separate the serializers.

See the README.txt in the 'c', 'cpp' and 'cpp2' directories for more details.

