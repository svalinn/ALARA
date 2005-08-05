#include <vector>
#include <iostream>

#include "FeindNs.h"
#include "Parent.h"
#include "LibDefine.h"
#include "RamLib.h"

#include "DecayEndf6.h"
#include "Eaf41.h"
#include "Cinder.h"
#include "EndfIeaf.h"

using namespace std;
using namespace FEIND;

#include "Elements.h"
#include "GroupStructs.h"

RamLib FEIND::Library;
FissionType FEIND::DefaultFT = NO_FISSION;

ErrCode FEIND::LoadLibrary(const LibDefine& lib)
{
  int err;

  switch(lib.Format)
    {
    case DECAY_ENDF_6:
      {
	DecayEndf6 data_lib(lib);
	err = data_lib.LoadLibrary();
      }
      break;
    case EAF_4_1:
      {
	Eaf41 data_lib(lib);
	err = data_lib.LoadLibrary();
      }
      break;
    case CINDER:
      {
	Cinder data_lib(lib);
	err = data_lib.LoadLibrary();
      }
      break;
    case ENDF_IEAF:
      {
	EndfIeaf data_lib(lib);
	err = data_lib.LoadLibrary();
      }
      break;
    default:
      return FEC_UNKNOWN_FORMAT;
      break;
    }


  return err;
}

Kza FEIND::DecayModetoKza(int decayMode, int dIso, Kza parent, Kza& sec)
{
  Kza ret;
  sec = 0;

  switch(decayMode)
    {
    case BETA_DECAY:
      // A_D = A_P, Z_D = Z_P+1:
      ret = parent + 10000;
      break;
    case ELECTRON_CAPTURE:
      // A_D = A_P, Z_D = Z_P-1:
      ret = parent - 10000;
      break;
    case ALPHA_DECAY:
      // A_D = A_P - 4, Z_D = Z_P-2:
      ret = (parent - 20040);
      sec = ALPHA;
      break;
    case NEUTRON_EMISSION:
      ret = (parent - 10);
      break;
    case PROTON_EMISSION:
      ret = (parent - 10010);
      sec = PROTON;
      break;
    case BETA_NEUTRON_EMIT:
      ret = (parent - 10 + 10000);
      break;
    case BETA_ALPHA_EMIT:
      ret = (parent + 10000 - 20040);
      sec = ALPHA;
      break;
    case POSITRON_ALPHA_EMIT:
      ret = (parent - 30040);
      sec = ALPHA;
      break;
    case SPONTANEOUS_FISSION:
      ret = 0;
      break;
    case ISOMERIC_TRANSITION:
      ret = (parent/10) * 10 + dIso;
      break;
    case POSITRON_PROTON_EMIT:
      ret = parent - 20010;
      sec = PROTON;
      break;
    default:
      // EXCEPTION: Unknown decay mode
      0;
    }

    // Take care of isomeric state transition:
    // NOTE: Although there is a case for this in the switch, we must include
    //       it again for cases where an isomeric transition accompanies
    //       another decay mode.
    ret = (ret/10) * 10 + dIso;

  return ret;
}

// bool FEIND::ParentExists(Kza parent)
// {
//   if(RamLib.find(parent) == RamLib.end())
//     return false;
  
//   return true;
// }
