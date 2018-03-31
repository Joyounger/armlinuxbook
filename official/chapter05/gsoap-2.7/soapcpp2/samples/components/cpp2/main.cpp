
// must include envH.h first: it declares the SOAP Header and Fault structures
// shared among the clients and services
#include "envH.h"

// include the quote and rate proxies and calc object
#include "quoteServiceProxy.h"
#include "rateServiceProxy.h"
#include "calcServiceService.h"

using namespace std;

int main(int argc, char *argv[])
{ if (argc <= 1)
  { calc::ServiceService calc;
    return calc.serve();
  }
  quote::ServiceProxy quote;
  float q;
  if (quote.getQuote(argv[1], q))
    soap_print_fault(&quote, stderr);
  else
  { if (argc > 2)
    { rate::ServiceProxy rate;
      float r;
      if (rate.getRate("us", argv[2], r))
        soap_print_fault(&rate, stderr);
      else
        q *= r;
    }
    cout << argv[1] << ": " << q << endl;
  }
  return 0;
}

namespace calc {

int ServiceService::add(double a, double b, double *result)
{ *result = a + b;
  return SOAP_OK;
}

int ServiceService::sub(double a, double b, double *result)
{ *result = a - b;
  return SOAP_OK;
}

int ServiceService::mul(double a, double b, double *result)
{ *result = a * b;
  return SOAP_OK;
}

int ServiceService::div(double a, double b, double *result)
{ if (b == 0.0)
  { int err = soap_sender_fault(this, "Division by zero", NULL);
    this->fault->detail = soap_new_SOAP_ENV__Detail(this, -1);
    soap_default_SOAP_ENV__Detail(this, this->fault->detail);
    this->fault->detail->f__error = soap_new__f__error(this, -1);
    this->fault->detail->f__error->problem = "Cannot divide by zero";
    this->fault->detail->f__error->a = a;
    this->fault->detail->f__error->b = b;
    return err;
  }
  *result = a / b;
  return SOAP_OK;
}

}
