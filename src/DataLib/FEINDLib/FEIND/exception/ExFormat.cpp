#include "ExFormat.h"

#include <string>
#include <sstream>

using namespace std;
using namespace FEIND;

ExFormat::ExFormat(const string& loc, FEINDFormat format) :
  Exception(loc, FEIND_FORMAT_ERROR)
{
  stringstream sstream;

  sstream <<"Format Number \"" << format << "\" is not supported in FEIND.\n\n";

  sstream << "The following format numbers are supported:\n"
	  << "  DECAY_ENDF_6     = 0\n"
	  << "  EAF_4_1          = 1\n"
	  << "  CINDER           = 2\n"
	  << "  ENDF_IEAF        = 3\n\n"
	  << "The symbols listed above should be used when interacting with FEIND, not \n"
	  << "the integers associated with them!\n";

  Detailed = sstream.str();
}
