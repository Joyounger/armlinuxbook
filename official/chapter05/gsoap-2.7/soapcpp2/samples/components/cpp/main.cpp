
// must include envH.h first: it declares the SOAP Header and Fault structures
// shared among the clients and services
#include "envH.h"

// include the quote and rate proxies and calc object
#include "quoteServiceProxy.h"
#include "rateServiceProxy.h"
#include "calcServiceObject.h"

// include the XML namespace mapping tables
#include "quote.nsmap"
#include "rate.nsmap"
#include "calc.nsmap"

using namespace std;

int main(int argc, char *argv[])
{ if (argc <= 1)
  { calc::Service calc;
    return calc.serve();
  }
  quote::Service quote;
  float q;
  if (quote.ns__getQuote(argv[1], q))
    soap_print_fault(quote.soap, stderr);
  else
  { if (argc > 2)
    { rate::Service rate;
      float r;
      if (rate.ns__getRate("us", argv[2], r))
        soap_print_fault(rate.soap, stderr);
      else
        q *= r;
    }
    cout << argv[1] << ": " << q << endl;
  }
  return 0;
}

namespace calc {

int ns__add(struct soap *soap, double a, double b, double *result)
{ *result = a + b;
  return SOAP_OK;
}

int ns__sub(struct soap *soap, double a, double b, double *result)
{ *result = a - b;
  return SOAP_OK;
}

int ns__mul(struct soap *soap, double a, double b, double *result)
{ *result = a * b;
  return SOAP_OK;
}

int ns__div(struct soap *soap, double a, double b, double *result)
{ if (b == 0.0)
  { int err = soap_sender_fault(soap, "Division by zero", NULL);
    soap->fault->detail = soap_new_SOAP_ENV__Detail(soap, -1);
    soap_default_SOAP_ENV__Detail(soap, soap->fault->detail);
    soap->fault->detail->f__error = soap_new__f__error(soap, -1);
    soap->fault->detail->f__error->problem = "Cannot divide by zero";
    soap->fault->detail->f__error->a = a;
    soap->fault->detail->f__error->b = b;
    return err;
  }
  *result = a / b;
  return SOAP_OK;
}

}
