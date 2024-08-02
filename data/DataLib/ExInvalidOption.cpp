#include "ExInvalidOption.h"

#include <sstream>

using namespace std;
using namespace FEIND;

ExInvalidOption::ExInvalidOption(const std::string& loc, 
				 const std::string& option) :
  Exception(loc, FEIND_INVALIDOPTION_ERROR)
{
  stringstream sstream;

  sstream << "Invalid option detected when initializing parser. The option " 
	  << "\"" << option << "\" is not\nsupported. Check the argument list "
	  << "that was passed to the parser for any\nmistakes.\n";

  Detailed = sstream.str();
}
