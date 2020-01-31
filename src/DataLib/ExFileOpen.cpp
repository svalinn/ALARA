#include "ExFileOpen.h"

using namespace std;
using namespace FEIND;

ExFileOpen::ExFileOpen(const std::string& loc, const std::string& fn) :
  Exception(loc, FEIND_FILEOPEN_ERROR)
{
  Detailed = "Unable to open file \"" + fn + "\". Check to make sure that \n"
    + "the file exists, and that you have permission to read it.\n";
}
