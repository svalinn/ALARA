#include "ExEmptyXSec.h"

using namespace FEIND;
using namespace std;

ExEmptyXSec::ExEmptyXSec(const string& loc) :
  Exception(loc, FEIND_EMPTYXSEC_ERROR)
{
  Detailed = 
    "\nAn illegal operation was performed on an empty cross section. Empty"
    "\ncross sections can not be used on the right hand side of"
    "\nexpressions. Functions such as XSec::Integrate() are not allowed as"
    "\nwell.\n\n";
}
