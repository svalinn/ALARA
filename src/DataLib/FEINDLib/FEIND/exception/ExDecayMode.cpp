#include "ExDecayMode.h"

#include <iostream>
#include <sstream>

using namespace FEIND;
using namespace std;

ExDecayMode::ExDecayMode(const string& loc, DecayModeType mode) :
  Exception(loc, FEIND_DECAYMODE_ERROR)
{
  stringstream sstream;

  sstream << "Decay mode number \"" << mode << "\" is not supported in FEIND.\n\n";

  sstream << "The following decay mode numbers are supported:\n"
	  << "GAMMA_DECAY          = 1\n"
	  << "BETA_DECAY           = 2\n"
	  << "ELECTRON_CAPTURE     = 3\n"
	  << "ISOMERIC_TRANSITION  = 4\n"
	  << "ALPHA_DECAY          = 5\n"
	  << "NEUTRON_EMISSION     = 6\n"
	  << "SPONTANEOUS_FISSION  = 7\n"
	  << "PROTON_EMISSION      = 8\n"
	  << "BETA_NEUTRON_EMIT    = 9\n"
	  << "BETA_ALPHA_EMIT      = 10\n"
	  << "POSITRON_ALPHA_EMIT  = 11\n"
	  << "POSITRON_PROTON_EMIT = 12\n"
	  << "IT_ALPHA_EMIT        = 13\n"
	  << "The symbols listed above should be used when interacting with FEIND, not \n"
	  << "the integers associated with them!\n";

  Detailed = sstream.str();
}
