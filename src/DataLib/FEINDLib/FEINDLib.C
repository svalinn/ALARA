#include "FEINDLib.h"
#include <math.h>

FEINDLib::FEINDLib(char* arg0, char* arg1, char* arg2, int setType)
  : DataLib(setType)
{
  FEIND::LibDefine lib;
  lib.Args.push_back(" ");

  int libCase = -9;

  if (strcmp(arg0,"EAF")==0)
    libCase = 1;
  else if (strcmp(arg0,"CINDER")==0)
    libCase = 2;

  switch (libCase)
  {
    case 1:
      //Load decay library
      lib.Format = FEIND::DECAY_ENDF_6;
      lib.Args[0] = arg1;
      //*** LOAD THE LIBRARY ***//
      try{
        FEIND::LoadLibrary(lib);
      } catch(FEIND::Exception& ex) {
        ex.Abort();
      }
    
      //Load activation library
      lib.Format = FEIND::EAF_4_1;
      nGroups = 175;
      lib.Args[0] = arg2;
      //*** LOAD THE LIBRARY ***//
      try{
        FEIND::LoadLibrary(lib);
      } catch(FEIND::Exception& ex) {
        ex.Abort();
      }
    break;
    case 2:
      lib.Format = FEIND::CINDER;
      nGroups = 63;
      lib.Args[0] = arg1;
      lib.Args.push_back(arg2);

      //*** LOAD THE LIBRARY ***//
      try{
        FEIND::LoadLibrary(lib);
      } catch(FEIND::Exception& ex) {
        ex.Abort();
      }
    break;
    default: 
    cerr << "*** ERROR LOADING NUCLEAR LIBRARY...\n"
	 << "Unknown nuclear data format \"" << arg0
	 << "\" encountered."
	 << "\n\nABORTING\n\n";
    exit(0);

  }

  nParents = (FEIND::Library.Parents()).size();
}

void FEINDLib::readData(int parent, NuclearData* data)
{
  int checkKza, nRxns=0;
  float thalf = 0, E[3] = {0,0,0};
  int *daughKza = NULL;
  char **emitted = NULL;
  float **xSection = NULL, *totalXSect=NULL;
  int rxnNum, emittedLen, numNZGrps, gNum;
  float bRatio;
  float decayConst = FEIND::Library.GetDecayConstant(parent);
  thalf = log(2.0)/decayConst;


  vector<int> daughterVec = FEIND::Library.Daughters(parent);

  nRxns = daughterVec.size();
  daughKza = new int[nRxns];

  xSection = new float*[nRxns];

  emitted = new char*[nRxns];

  for (rxnNum = 0; rxnNum < nRxns; rxnNum++)
  {
    daughKza[rxnNum] = daughterVec[rxnNum];

    //Create emitted[rxnNum]. Currently, FEIND cannot handle types of reaction.
    emitted[rxnNum] = new char[3+1];
    emitted[rxnNum][0] = 'x';
    emitted[rxnNum][1] = 'x';
    emitted[rxnNum][2] = 'x';
    emitted[rxnNum][3] = '\0';

   //Create xSection[rxnNum] with a size of nGroups+1  
    xSection[rxnNum] = new float[nGroups+1];
    
    FEIND::XSec csc = FEIND::Library.GetDCs(parent, daughterVec[rxnNum], FEIND::TOTAL_CS);
    for (gNum = 0; gNum < nGroups; gNum++)
      xSection[rxnNum][gNum] = csc[gNum];

    bRatio = FEIND::Library.GetBratio(parent, daughterVec[rxnNum]);
  
    xSection[rxnNum][nGroups] = bRatio*decayConst;

			      
  }  

  
  E[0] = FEIND::Library.GetDecayEnergy(parent, FEIND::LIGHT_PARTICLES); 
  E[1] = FEIND::Library.GetDecayEnergy(parent, FEIND::EM_RADIATION);
  E[2] = FEIND::Library.GetDecayEnergy(parent, FEIND::HEAVY_PARTICLES);

  data->setData(nRxns,E,daughKza,emitted,xSection,thalf,totalXSect);

  for (rxnNum=0;rxnNum<nRxns;rxnNum++)
    {
      delete xSection[rxnNum];
      delete emitted[rxnNum];
    }
  delete xSection;
  delete emitted;
  delete daughKza;

  xSection = NULL;
  emitted = NULL;
  daughKza = NULL;

}

void FEINDLib::readGammaData(int parent, GammaSrc* gsrc)
{


}
