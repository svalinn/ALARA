#include <vector>
#include <iostream>
#include <string>
#include <cassert>

#include "FeindNs.h"
#include "Parent.h"
#include "LibDefine.h"
#include "RamLib.h"

#include "Parser.h"
#include "DecayEndf6.h"
#include "Eaf41.h"
#include "Cinder.h"
#include "EndfIeaf.h"

using namespace std;
using namespace FEIND;

#include "Elements.h"
#include "exception/ExInclude.h"

RamLib FEIND::Library;
FissionType FEIND::DefaultFT = NO_FISSION;

const XSec FEIND::NULLCS;
const vector<Kza> FEIND::EMPTY_VEC_KZA;
const vector<pair<double,double> > FEIND::EMPTY_PAIR_DOUBLE;

void FEIND::LoadLibrary(const LibDefine& lib)
{
  Parser* p_parser = NULL;

  try{
    switch(lib.Format)
      {
      case DECAY_ENDF_6:
	p_parser = new DecayEndf6(lib);
	break;
      case EAF_4_1:
	p_parser = new Eaf41(lib);
	break;
      case CINDER:
	p_parser = new Cinder(lib);
	break;
      case ENDF_IEAF:
	p_parser = new EndfIeaf(lib);
	break;
      default:
	throw ExFormat("Global FEIND::LoadLibrary function", lib.Format);
      }
  
    assert(p_parser);

    p_parser->LoadLibrary();
 
  } catch (Exception& ex) {
    if(p_parser) delete p_parser;
    throw;
  }

  delete p_parser;
}

Kza FEIND::DecayModetoKza(DecayModeType decayMode, int dIso, Kza parent, 
			  Kza& sec)
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
    case IT_ALPHA_EMIT:
      ret = parent - 20040;
      sec = ALPHA;
      break;
    default:
      throw ExDecayMode("FEIND::DecayModeToKza() function",  decayMode);
    }

    // Take care of isomeric state transition:
    // NOTE: Although there is a case for this in the switch, we must include
    //       it again for cases where an isomeric transition accompanies
    //       another decay mode.
    ret = (ret/10) * 10 + dIso;

  return ret;
}
