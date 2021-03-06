
Thanks for using gSOAP!

gSOAP provides a cross-platform development toolkit for developing server,
client, and peer Web service applications in C and C++.

* The gSOAP 'soapcpp2' compiler and 'stdsoap2' runtime are stable since
  version release 2.1.3.

* The gSOAP 'wsdl2h' WSDL parser is stable since wsdl2h version release 1.1.0.
  The WSDL parser fully supports WSDL 1.1 and XML schemas.

The software is provided "as is", without any warranty.  However, gSOAP
has received a lot of support from users and has been extensively tested
in the real world.  We also continue to improve gSOAP and add new features.

WHAT'S COOL?

The gSOAP WSDL parser is a gSOAP application itself, which demonstrates the
capabilities of the generic XML handling by the toolkit to parse WSDL, XML
schemas, and SOAP/XML.

gSOAP supports streaming technologies to expedite SOAP/XML and MTOM/MIME and
DIME attachment transfers.  It is the only toolkit that supports streaming
MIME and DIME transfers of potentially unlimited data lengths.

gSOAP is the only toolkit that supports the serialization of native C and C++
data types.  You can use it to export and import your application data in XML
without having to write wrapper routines.

gSOAP ensures as small memory footprint, because XML is a processed as a
transient format and not buffered.  Many optimizations have been applied to
reduce resource requirements and to expedite XML parsing.

INSTALLATION

See NOTES.txt for distribution notes and installation instructions.

See the 'bin' directory for soapcpp2 and wsdl2h executables.

See the 'doc' directory doc/index.html for documentation.

See the 'samples' directory for example gSOAP Web service applications.

See the 'uddi2' directory to build UDDI v2 registries.

See the 'WS' directory for WS-* protocol support. This part of the software is
under development as new WS-* protocols are published.

See the 'import' directory for WS-* protocols you can import.

See the 'plugin' directory for plugins.

See the 'custom' directory for example custom serializers.

See the 'mod_gsoap' directory for Apache mod_gsoap, IIS modules, and WinInet.

See the 'extras' directory with third-party contributions.

GETTING STARTED

The gSOAP WSDL parser converts WSDL into a gSOAP header file for processing
with the gSOAP stub and skeleton compiler to build your Web services
applications.  You can use the WSDL parser to translate WSDL and/or XML schemas
into C or C++ data structures and XML parsers. You can also use the gSOAP
compiler separately to create XML serialization routines for application data.

For example:

$ wsdl2h -s -o XMethodsQuery.h http://www.xmethods.net/wsdl/query.wsdl

$ soapcpp2 XMethodsQuery.h

The XMethodsQuery.h header file contains a translation of the services and XML
schemas to C/C++ and other useful information copied from the WSDL.  The
header file is then processed by the gSOAP stub and skeleton compiler to
generate the following files:

soapClient.cpp		client-side stub routines for service invocation
soapServer.cpp		server-side skeleton routines for server development
soapC.cpp		C/C++ parameter marshalling code

To develop a C++ client application, you can also use the generated
'soapXMethodsQuerySoapProxy.h' class and 'XMethodsQuerySoap.nsmap' XML
namespace table to access the XMethods Web service. Both need to be
'#include'd in your source. Then compile and link the soapC.cpp,
soapClient.cpp, and stdsoap2.cpp sources to complete the build.

To develop a C client, use wsdl2h option -c. The emitted source code will be
in C.

More information about the wsdl2h and soapcpp2 tools and their options can be
found in the gSOAP documentation and the Quick How-To page on the gSOAP Web
site, see: http://gsoap2.sourceforge.net

See also the 'wsdl/README.txt' for more details on the WSDL parser and
installation (in case you don't have the wsdl2h executable).

LICENSE

gSOAP is distributed with a choice of three licenses:

* The gSOAP public open source license (which is based on the Mozilla public
  license 1.1). See license.html or license.pdf for further details.

* Or GPL (GNU Public License, a common open-source software license).

* Or a proprietary software development license.

You can choose which license applies best for your intended use. The software
is offered under GPL conditions to enable the software to be used with open
source GPL projects, educational use, and so on. We do not accept third-party
GPL contributions to avoid having to fork the code base in GPL and non-GPL.

IMPORTANT: the wsdl2h WSDL parser, UDDI code, and sample applications such as
the stand-alone web server are distributed ONLY under the GPL or the
proprietary license.  This means that commercial use of wsdl2h to generate
code for product development, or the use of the UDDI source code, or the use
of code from the sample Web service applications requires the proprietary
software development license. The reason for this is that the wsdl2h tool,
UDDI and some of the sample applications are not developed at Florida State
University. Please contact me by email if you have any questions on licensing
and software support: engelen AT acm DOT org.

The parts of the code that are strictly distributed under the GPL (i.e. the
code that is only distributed under GPL and not jointly under GPL and gSOAP
public license) cannot be used for commercial purposes. These parts are:

* The wsdl2h WSDL parser source code and the code generated by it.

* The UDDI source code.

* The examples included in the gSOAP distribution package 'samples' directory.

For more information, visit:

http://www.cs.fsu.edu/~engelen/soaplicense.html

A commercial license is available for the GPL licensed software. Please visit:

http://www.genivia.com/Products/gsoap/contract.html

COPYRIGHT

gSOAP is copyrighted by Robert A. van Engelen, Genivia, Inc.
Copyright (C) 2000-2005 Robert A. van Engelen, Genivia, Inc.
All Rights Reserved.

USE RESTRICTIONS

You may not: (i) transfer rights to gSOAP or claim authorship; or (ii) remove
any product identification, copyright, proprietary notices or labels from gSOAP.

WARRANTY 

GENIVIA INC. EXPRESSLY DISCLAIMS ALL WARRANTIES, WHETHER EXPRESS, IMPLIED OR
STATUTORY, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, OF FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT OF THIRD
PARTY INTELLECTUAL PROPERTY RIGHTS, AND ANY WARRANTY THAT MAY ARISE BY REASON
OF TRADE USAGE, CUSTOM, OR COURSE OF DEALING.  WITHOUT LIMITING THE
FOREGOING, YOU ACKNOWLEDGE THAT THE SOFTWARE IS PROVIDED "AS IS" AND THAT
GENIVIA INC. DO NOT WARRANT THE SOFTWARE WILL RUN UNINTERRUPTED OR ERROR FREE.
LIMITED LIABILITY: THE ENTIRE RISK AS TO RESULTS AND PERFORMANCE OF THE
SOFTWARE IS ASSUMED BY YOU.  UNDER NO CIRCUMSTANCES WILL GENIVIA INC. BE LIABLE
FOR ANY SPECIAL, INDIRECT, INCIDENTAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES OF
ANY KIND OR NATURE WHATSOEVER, WHETHER BASED ON CONTRACT, WARRANTY, TORT
(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, ARISING OUT OF OR IN
ANY WAY RELATED TO THE SOFTWARE, EVEN IF GENIVIA INC. HAS BEEN ADVISED ON THE
POSSIBILITY OF SUCH DAMAGE OR IF SUCH DAMAGE COULD HAVE BEEN REASONABLY
FORESEEN, AND NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
EXCLUSIVE REMEDY PROVIDED.  SUCH LIMITATION ON DAMAGES INCLUDES, BUT IS NOT
LIMITED TO, DAMAGES FOR LOSS OF GOODWILL, LOST PROFITS, LOSS OF DATA OR
SOFTWARE, WORK STOPPAGE, COMPUTER FAILURE OR MALFUNCTION OR IMPAIRMENT OF
OTHER GOODS.  IN NO EVENT WILL GENIVIA INC. BE LIABLE FOR THE COSTS OF
PROCUREMENT OF SUBSTITUTE SOFTWARE OR SERVICES.  YOU ACKNOWLEDGE THAT THIS
SOFTWARE IS NOT DESIGNED FOR USE IN ON-LINE EQUIPMENT IN HAZARDOUS
ENVIRONMENTS SUCH AS OPERATION OF NUCLEAR FACILITIES, AIRCRAFT NAVIGATION OR
CONTROL, OR LIFE-CRITICAL APPLICATIONS.  GENIVIA INC. EXPRESSLY DISCLAIM ANY
LIABILITY RESULTING FROM USE OF THE SOFTWARE IN ANY SUCH ON-LINE EQUIPMENT IN
HAZARDOUS ENVIRONMENTS AND ACCEPTS NO LIABILITY IN RESPECT OF ANY ACTIONS OR
CLAIMS BASED ON THE USE OF THE SOFTWARE IN ANY SUCH ON-LINE EQUIPMENT IN
HAZARDOUS ENVIRONMENTS BY YOU.  FOR PURPOSES OF THIS PARAGRAPH, THE TERM
"LIFE-CRITICAL APPLICATION" MEANS AN APPLICATION IN WHICH THE FUNCTIONING OR
MALFUNCTIONING OF THE SOFTWARE MAY RESULT DIRECTLY OR INDIRECTLY IN PHYSICAL
INJURY OR LOSS OF HUMAN LIFE.

LIBRARIES

gSOAP is self-contained and does not require any third-party software, except
for the Zlib and OpenSSL libraries that must be installed to support
compression and encryption. Compression and encryption are optional.

To build the gSOAP 'soapcpp2' compiler, you must have Bison and Flex installed
(or the older Yacc and Lex equivalents).

Win32 build of clients and services requires winsock.dll. To do this in
Visual C++ 6.0, go to "Project", "settings", select the "Link" tab (the
project file needs to be selected in the file view) and add "wsock32.lib" to
the "Object/library modules" entry. The Win32 distribution contains two
MSVC++ project examples. The custom build in VC++ 6.0 has been configured to
invoke the gSOAP compiler automatically. The VC++ projects can be converted to
MSVC++ 7.0.

DISCLAIMER

WE TRY OUR BEST TO PROVIDE YOU WITH "REAL-WORLD" EXAMPLES BUT WE CANNOT
GUARANTEE THAT ALL CLIENT EXAMPLES CAN CONNECT TO THIRD PARTY WEB SERVICES
WHEN THESE SERVICES ARE DOWN OR HAVE BEEN REMOVED.

