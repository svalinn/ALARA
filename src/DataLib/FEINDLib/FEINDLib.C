#include "FEINDLib.h"
#include <math.h>

FEINDLib::FEINDLib(vector<string> filePath,vector<string> fileFormat,int setType) 
  : DataLib(setType)
{
  FEIND::LibDefine lib;
  lib.push_back(" ");
  
  for (int idx=0; idx < filePath.size(); idx++)
  {
    
    string format_str = fileFormat[idx];

     //*** SET THE FORMAT ***//

    if(format_str == "DECAY_ENDF_6")
      lib.Format = FEIND::DECAY_ENDF_6;
    else if(format_str == "EAF_4_1")
      {lib.Format = FEIND::EAF_4_1; nGroups = 175;}
    else if(format_str == "CINDER")
      {lib.Format = FEIND::CINDER; nGroups = 63;}
    else
      {
        cerr << "*** ERROR LOADING NUCLEAR LIBRARY...\n"
	   << "Unknown nuclear data format \"" << format_str
	   << "\" encountered."
	   << "\n\nABORTING\n\n";
        assert(0);
      }
     
    lib.Args[0] = filePath[idx];

  //*** LOAD THE LIBRARY ***//
  try{
    FEIND::LoadLibrary(lib);
  } catch(FEIND::Exception& ex) {
    ex.Abort();
  }

  }

  nParents = (FEIND::Library.Parents()).size();

}

void FEINDLib::readData(int parent, NuclearData* data)
{
  int checkKza, nRxns=0;
  float thalf = 0, E[3] = {0,0,0};
  int *daugKza = NULL;
  char **emitted = NULL;
  float **xSection = NULL, *totalXSect=NULL;
  int rxnNum, emittedLen, numNZGrps, gNum;
  float bRatio;
  float decayConst = FEIND::GetDecayConstant(parent);
  thalf = log(2)/decayConst;


  vector<int> daughterVec = FEIND::Daughters(parent);

  nRxns = daughterVec.size();
  daughKza = new int[nRxns];

  xSection = new float*[nRxns];

  emitted = new char*[nRxns];

  for (rxnNum = 0; rxnNum < nRxns; rxnNum++)
  {
    *daughKza = daughterVec[rxnNum];
    daughKza++;

    //Create emitted[rxnNum]. Currently, FEIND cannot handle types of reaction.
    emitted[rxnNum] = new char[3+1];
    emitted[rxnNum][0] = 'x';
    emitted[rxnNum][1] = 'x';
    emitted[rxnNum][2] = 'x';
    emitted[rxnNum][3] = '\0';

   //Create xSection[rxnNum] with a size of nGroups+1  
    xSection[rxnNum] = new float[nGroups+1];
    
    FEIND::XSec csc = FEIND::GetDCs(parent, daughterVec[rxnNum], FEIND::TOTAL_CS);
    for (gNum = 0; gNum < nGroups; gNum++)
      xSection[gNum] = csc[gNum];

    bRatio = FEIND::GetBratio(parent, daughterVec[rxnNum];
  
    xSection[nGroups] = bRatio*decayConst;

    //HAVE TO WORK ON totalXSect
    totalXSect = new float[nGroups+1];

    FEIND::XSec tCsc = FEIND::GetPCs(parent, FEIND::TOTAL_CS);
    for (gNum = 0; gNum < nGroups; gNum++)
      totalXSect[gNum] = tCsc[gNum];
     
    totalXSec[nGroups] = decayConst;
			      
  }  

  
  E[0] = FEIND::GetDecayEnergy(parent, LIGHT_PARTICLES); 
  E[1] = FEIND::GetDecayEnergy(parent, EM_RADIATION);
  E[2] = FEIND::GetDecayEnergy(parent, HEAVY_PARTICLES);

  data->setData(nRxns,E,daugKza,emitted,xSection,thalf,totalXSect);

  for (rxnNum=0;rxnNum<nRxns;rxnNum++)
    {
      delete xSection[rxnNum];
      delete emitted[rxnNum];
    }
  delete xSection;
  delete emitted;
  delete daugKza;
  delete totalXSect;

  xSection = NULL;
  emitted = NULL;
  daugKza = NULL;
  totalXSect = NULL;

}

void FEINDLib::readGammaData(int parent, GammSrc* gsrc)
{


}
