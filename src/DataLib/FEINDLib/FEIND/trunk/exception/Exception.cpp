#include "Exception.h"

#include <iostream>
#include <cstdlib>

using namespace std;
using namespace FEIND;

Exception::Exception(const string& loc, FEINDErrorType ec) :
  Location(loc)
{
  ErrorCode = ec;
}

void Exception::Print() const
{
  cerr << "\n\n*** FEIND ERROR ENCOUNTERED ***\n\n" 
       << "FEIND has encounterred an error at:\n"
       << Location << endl << endl
       << "Detailed Error Message:\n"
       << Detailed << endl << endl;
}

void Exception::Abort() const
{
  Print();
  cerr << "*** ABORTING PROGRAM WITH ERROR CODE " << ErrorCode << " ***\n\n";
  exit(ErrorCode);
}
