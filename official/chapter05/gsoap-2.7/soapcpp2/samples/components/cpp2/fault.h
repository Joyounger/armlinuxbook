/*	fault.h

	Defines optional SOAP Fault derail data structures

	Copyright (C) 2000-2004 Robert A. van Engelen. All Rights Reserved.

*/

/*

Add any data structure you want to serialize as part of the SOAP Fault detail
element. The detail element '__type' and 'fault' fields should be set to
transmit data. The fields are set when data of corresponding types are received.

For example, we define an <element> of name <f:myData> with a string vector
(note the leading _ in the following declaration):
*/


//gsoap f schema namespace: urn:calc.fault
//gsoap f schema elementForm: qualified
//gsoap f schema attributeForm: unqualified

class _f__error
{ public:
  std::string problem;
  double a;
  double b;
};

/*
In addition, you can modify the SOAP_ENV__Detail struct and add your own set of
namespace qualified fields, as in:
*/

struct SOAP_ENV__Detail
{ _f__error *f__error;
  int __type;
  void *fault;
  _XML __any;
};
 
/*
To return a fault from your service application:

soap_sender_fault(soap, "An error occurred", NULL));	// set soap fault
soap->fault->detail = (struct SOAP_ENV__Detail*)soap_malloc(soap, sizeof(struct SOAP_ENV__Fault));
soap->fault->f__myData = soap_new__f__myData(soap, -1);
return SOAP_FAULT;
*/
