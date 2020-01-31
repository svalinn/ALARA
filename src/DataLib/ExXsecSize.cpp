#include "ExXsecSize.h"

#include <sstream>

using namespace std;
using namespace FEIND;

ExXsecSize::ExXsecSize(const std::string& loc, int xsSize, int otherSize) :
  Exception(loc, FEIND_XSECSIZE_ERROR)
{
  stringstream sstream;

  sstream << "Size mismatch when performing operation involving a XSec object. The "
	  << "XSec object\nhas size " << xsSize << " and is involved in a calculation "
	  << "with an object of size " << otherSize << ".\n";

  Detailed = sstream.str();
}
