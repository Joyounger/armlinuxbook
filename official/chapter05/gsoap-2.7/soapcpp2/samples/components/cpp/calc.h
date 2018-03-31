namespace calc {

//gsoap ns service name:	Service
//gsoap ns service style:	rpc
//gsoap ns service encoding:	encoded
//gsoap ns service location:	http://www.cs.fsu.edu/~engelen/calc.cgi
//gsoap ns service namespace:	urn:calc

//gsoap f schema namespace: urn:calc.fault
//gsoap f schema import: f.xsd

// _f__error is a fault element defined in fault.h (in the standard namespace)
extern class _f__error;

int ns__add(double a, double b, double *result);
int ns__sub(double a, double b, double *result);
int ns__mul(double a, double b, double *result);
//gsoap ns service method-fault: div f__error
int ns__div(double a, double b, double *result);

}
