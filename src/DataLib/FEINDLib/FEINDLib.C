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
      initFissionType(arg2);
      //lib.Args.push_back("noyields");

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
  float **xSection = NULL, *totalXSect = NULL;
  int rxnNum, emittedLen, numNZGrps, gNum;
  double fy,sfy;
  float bRatio;
  float decayConst = FEIND::Library.GetDecayConstant(parent);
  thalf = log(2.0)/decayConst;

  double sfbr = FEIND::Library.GetSfbr(parent);
  FEIND::XSec fission_xs = FEIND::Library.GetPCs(parent,FEIND::NEUTRON_FISSION_CS);

  vector<int> daughterVec = FEIND::Library.Daughters(parent);

  nRxns = daughterVec.size();

  daughKza = new int[nRxns];

  xSection = new float*[nRxns];

  emitted = new char*[nRxns];

  //Create totalXSect
  totalXSect = new float[nGroups+1];
  FEIND::XSec totalCsc = FEIND::Library.GetPCs(parent, FEIND::TOTAL_CS);

    try{

      if(totalCsc)
	{
	  for (gNum = 0; gNum < nGroups; gNum++)
	    {
	      totalXSect[gNum] =  totalCsc[gNum]; 
	    }
	}
      else
	{
	  for (gNum = 0; gNum < nGroups; gNum++)
	    {
	      totalXSect[gNum] =  0.0; 
	    }
	}

    } catch(FEIND::Exception& ex)
      {
	ex.Abort();
      }

  totalXSect[nGroups] = decayConst;

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
   
    //Modify csc if this parent-daughterVec[rxnNum] path is a fission path.
    //If fission yield does not exist, fy will be zero.    
    try{

      if(csc)
	{
	  for (gNum = 0; gNum < nGroups; gNum++)
	    {
	      xSection[rxnNum][gNum] =  csc[gNum]; 
	    }
	}
      else
	{
	  for (gNum = 0; gNum < nGroups; gNum++)
	    {
	      xSection[rxnNum][gNum] =  0.0; 
	    }
	}
  
      xSection[rxnNum][nGroups] = 0.0;

    } catch(FEIND::Exception& ex)
      {
	ex.Abort();
      }

    fy = FEIND::Library.GetFissionYield(parent,daughterVec[rxnNum], fissionType);

    if (fy != 0)
     {     
       for (gNum = 0; gNum < nGroups; gNum++)
         xSection[rxnNum][gNum] += fy*fission_xs[gNum];                
     }

    if (decayConst != 0)
    {
      if (sfbr != 0 )
      {
        //Check for spontaneous fission
        sfy = FEIND::Library.GetFissionYield(parent,daughterVec[rxnNum], FEIND::FISSION_SF);  
        xSection[rxnNum][nGroups] = decayConst*sfy*sfbr;
      }
      else 
      {  
        bRatio = FEIND::Library.GetBratio(parent, daughterVec[rxnNum]);  
        xSection[rxnNum][nGroups] = bRatio*decayConst;
      }
    }


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
  delete totalXSect;

  xSection = NULL;
  emitted = NULL;
  daughKza = NULL;
  totalXSect = NULL;

}

void FEINDLib::readGammaData(int parent, GammaSrc* gsrc)
{
  
}

void FEINDLib::initFissionType(char* arg2)
{

  if (strcmp(arg2,"NO_FISSION")==0)
    fissionType = NO_FISSION;
  else if (strcmp(arg2,"FAST")==0)
    fissionType = FAST;
  else if (strcmp(arg2,"THERMAL")==0)
    fissionType = THERMAL;
  else if (strcmp(arg2,"HOT")==0)
    fissionType = HOT;
  else if (strcmp(arg2,"SF")==0)
    fissionType = SF;  

}

