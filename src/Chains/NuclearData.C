/* File sections:
 * Service: constructors, destructors
 * Chain: functions directly related to the building and analysis of chains
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "NuclearData.h"
#include "truncate.h"

#include "Calc/VolFlux.h"

#include "DataLib/DataLib.h"

/***************************
 ********* Service *********
 **************************/

int NuclearData::nGroups = 0;
DataLib* NuclearData::dataLib = NULL;

/* basic constructor for NuclearData base class */
NuclearData::NuclearData()
{
  nPaths=-1;
  relations=NULL;
  emitted=NULL;
  single=NULL;
  paths=NULL;
  P=NULL;
  D=NULL;
  for (int dHeat=0;dHeat<3;dHeat++)
    E[dHeat] = 0;
}

NuclearData::NuclearData(const NuclearData& n)
{

  int rxnNum,gNum;
  
  nPaths = n.nPaths;

  relations=NULL;
  emitted=NULL;
  single=NULL;
  paths=NULL;
  P=NULL;
  D=NULL;
  for (int dHeat=0;dHeat<3;dHeat++)
    E[dHeat] = 0;

  if (nPaths>0)
    {

      relations = new int[nPaths];
      memCheck(relations,"NuclearData::NuclearData(...) copy constructor: relations");
      
      emitted = new char*[nPaths];
      memCheck(emitted,"NuclearData::NuclearData(...) copy constructor: emitted");
      
      paths = new double*[nPaths+1];
      memCheck(paths,"NuclearData::NuclearData(...) copy constructor: paths");
      
      paths[nPaths] = new double[nGroups+1];
      memCheck(paths[rxnNum],"NuclearData::NuclearData(...) copy constructor: paths[n]");
      single = new double[nGroups+1];
      memCheck(single,"NuclearData::NuclearData(...) copy constructor: single");
      
      for (rxnNum=0;rxnNum<nPaths;rxnNum++)
	{
	  relations[rxnNum] = n.relations[rxnNum];
	  
	  emitted[rxnNum] = new char[strlen(n.emitted[rxnNum])+1];
	  memCheck(emitted[rxnNum],"NuclearData::NuclearData(...) copy constructor: emitted[n]");
	  strcpy(emitted[rxnNum],n.emitted[rxnNum]);
	  
	  paths[rxnNum] = new double[nGroups+1];
	  memCheck(paths[rxnNum],"NuclearData::NuclearData(...) copy constructor: paths[n]");
	  for (gNum=0;gNum<=nGroups;gNum++)
	    paths[rxnNum][gNum] = n.paths[rxnNum][gNum];

	  if (n.D == n.paths[rxnNum])
	    {
	      D = paths[rxnNum];
	      P = single;
	    }
	  if (n.P == n.paths[rxnNum])
	    {
	      P = paths[rxnNum];
	      D = single;
	    }
	}
      
      for (gNum=0;gNum<=nGroups;gNum++)
	{
	  paths[rxnNum][gNum] = n.paths[rxnNum][gNum];
	  single[gNum] = n.single[gNum];
	}
      
      if (n.D == n.paths[rxnNum])
	{
	  D = paths[rxnNum];
	  P = single;
	}
      if (n.P == n.paths[rxnNum])
	{
	  P = paths[rxnNum];
	  D = single;
	}

      E[0] = n.E[0];
      E[1] = n.E[1];
      E[3] = n.E[2];
    }


}  

NuclearData::NuclearData(double *sigmaP)
{
  nPaths=-1;
  relations=NULL;
  emitted=NULL;
  single=NULL;
  paths=NULL;
  P=NULL;
  D=NULL;
  for (int dHeat=0;dHeat<3;dHeat++)
    E[dHeat] = 0;

  /* DELETED in destructor for class NuclearData called by destructor 
     for class Node */
  single = new double[nGroups+1];
  memCheck(single,"NuclearData::NuclearData(...): single");
  for (int gNum=0;gNum<=nGroups;gNum++)
    single[gNum] = sigmaP[gNum];
  P = single;
}

/* basic destructor for NuclearData */
NuclearData::~NuclearData()
{
  if (nPaths>0)
    {

      int rxnNum;

      delete relations;
      delete single;
      for (rxnNum=0;rxnNum<nPaths;rxnNum++)
	{
	  delete emitted[rxnNum];
	  delete paths[rxnNum];
	}
      delete paths[rxnNum];
      delete emitted;
      delete paths;
      relations = NULL;
      emitted=NULL;
      single=NULL;
      paths=NULL;
      P = NULL;
      D = NULL;
    }

  nPaths = -1;
}

NuclearData& NuclearData::operator=(const NuclearData& n)
{

  if (this == &n)
    return *this;

  int rxnNum,gNum;
  
  nPaths = n.nPaths;

  delete relations;
  delete single;
  for (rxnNum=0;rxnNum<nPaths;rxnNum++)
    {
      delete emitted[rxnNum];
      delete paths[rxnNum];
    }
  delete paths[rxnNum];
  delete emitted;
  delete paths;

  relations=NULL;
  emitted=NULL;
  single=NULL;
  paths=NULL;
  P=NULL;
  D=NULL;
  for (int dHeat=0;dHeat<3;dHeat++)
    E[dHeat] = 0;

  if (nPaths>0)
    {

      relations = new int[nPaths];
      memCheck(relations,"NuclearData::NuclearData(...) copy constructor: relations");
      
      emitted = new char*[nPaths];
      memCheck(emitted,"NuclearData::NuclearData(...) copy constructor: emitted");
      
      paths = new double*[nPaths+1];
      memCheck(paths,"NuclearData::NuclearData(...) copy constructor: paths");
      
      paths[nPaths] = new double[nGroups+1];
      memCheck(paths[nPaths],"NuclearData::NuclearData(...) copy constructor: paths[n]");
      single = new double[nGroups+1];
      memCheck(single,"NuclearData::NuclearData(...) copy constructor: single");
      
      for (rxnNum=0;rxnNum<nPaths;rxnNum++)
	{
	  relations[rxnNum] = n.relations[rxnNum];
	  
	  emitted[rxnNum] = new char[strlen(n.emitted[rxnNum])+1];
	  memCheck(emitted[rxnNum],"NuclearData::NuclearData(...) copy constructor: emitted[n]");
	  strcpy(emitted[rxnNum],n.emitted[rxnNum]);
	  
	  paths[rxnNum] = new double[nGroups+1];
	  memCheck(paths[rxnNum],"NuclearData::NuclearData(...) copy constructor: paths[n]");
	  for (gNum=0;gNum<=nGroups;gNum++)
	    paths[rxnNum][gNum] = n.paths[rxnNum][gNum];

	  if (n.D == n.paths[rxnNum])
	    {
	      D = paths[rxnNum];
	      P = single;
	    }
	  if (n.P == n.paths[rxnNum])
	    {
	      P = paths[rxnNum];
	      D = single;
	    }

	}
      
      for (gNum=0;gNum<=nGroups;gNum++)
	{
	  paths[rxnNum][gNum] = n.paths[rxnNum][gNum];
	  single[gNum] = n.single[gNum];
	}
      
      if (n.D == n.paths[rxnNum])
	{
	  D = paths[rxnNum];
	  P = single;
	}
      if (n.P == n.paths[rxnNum])
	{
	  P = paths[rxnNum];
	  D = single;
	}

      E[0] = n.E[0];
      E[1] = n.E[1];
      E[3] = n.E[2];
    }


  return *this;


}  

/***************************
 *********** Input *********
 ***************************/

void NuclearData::getDataLib(istream& input)
{
  char type[64];

  input >> type;
  verbose(2,"Openning DataLib with type %s",type);
  dataLib = DataLib::newLib(type,input);

  nGroups = dataLib->getNumGroups();

  VolFlux::setNumGroups(nGroups);
	  
}

void NuclearData::closeDataLib()
{ 
  delete dataLib; 
}

/****************************
 ********** Chain ***********
 ***************************/

/* set the nuclear data with arguments passed from dataLib routine */
void NuclearData::setData(int numRxns, float* radE, int* daugKza, 
			  char** emissions, float** xSection, float thalf)
{
  int gNum, rxnNum;

  verbose(4,"Setting NuclearData members.");

  nPaths = numRxns;

  delete relations;
  delete [] emitted;
  delete [] paths;

  relations = new int[nPaths];
  memCheck(relations,"NuclearData::setData(...) : relations");

  emitted = new char*[nPaths];
  memCheck(emitted,"NuclearData::setData(...) : emitted");

  paths = new double*[nPaths+1];
  memCheck(paths,"NuclearData::setData(...) : paths");

  E[0] = radE[0];  
  E[1] = radE[1];  
  E[2] = radE[2];

  /* initialize total x-section */
  paths[nPaths] = new double[nGroups+1];
  memCheck(paths[nPaths],"NuclearData::setData(...) : paths[n]");
  for (gNum=0;gNum<nGroups;gNum++)
    paths[nPaths][gNum] = 0;

  if (thalf>0)
    paths[nPaths][nGroups] = log(2.0)/thalf;
  else
    paths[nPaths][nGroups] = 0;

  /* setup each reaction */
  for (rxnNum=0;rxnNum<nPaths;rxnNum++)
    {
      debug(4,"Copying reaction %d with %d groups.",rxnNum,nGroups+1);
      relations[rxnNum] = daugKza[rxnNum];

      emitted[rxnNum] = new char[strlen(emissions[rxnNum])+1];
      memCheck(emitted[rxnNum],"NuclearData::setData(...) : emitted[n]");
      strcpy(emitted[rxnNum],emissions[rxnNum]);

      paths[rxnNum] = new double[nGroups+1];
      memCheck(paths[rxnNum],"NuclearData::setData(...) : paths[n]");

      for (gNum=0;gNum<nGroups;gNum++)
	{
	  paths[rxnNum][gNum] = xSection[rxnNum][gNum]*1e-24;
	  if (strcmp(emitted[rxnNum],"x"))
	    paths[nPaths][gNum] += paths[rxnNum][gNum];
	}
      paths[rxnNum][nGroups] = xSection[rxnNum][nGroups];
    }

}

/* strip pure transmutation reactions out of data */
int NuclearData::stripNonDecay()
{
  int rxnNum = 0;

  /* count decay reactions */
  int numDecay = 0;
  for (rxnNum=0;rxnNum<nPaths;rxnNum++)
    if (paths[rxnNum][nGroups]>0)
      numDecay++;

  /* if there are fewer decay reactions than the total number, 
   * copy them to a new array */
  if (numDecay < nPaths)
    {
      int decayRxnNum = 0;

      int* newDaug = new int[numDecay];
      memCheck(newDaug,"NuclearData::stripNonDecay(...): newDaug");

      double **newPaths = new double*[numDecay+1];
      memCheck(newPaths,"NuclearData::stripNonDecay(...): newPaths");

      char **newEmitted = new char*[numDecay];
      memCheck(newEmitted,"NuclearData::stripNonDecay(...): newEmitted");

      /* if we have any decay reactions
       * find them and copy them */
      if (numDecay > 0)
	for (rxnNum=0;rxnNum<nPaths;rxnNum++)
	  if (paths[rxnNum][nGroups]>0)
	    {
	      newDaug[decayRxnNum] = relations[rxnNum];
	      newPaths[decayRxnNum] = paths[rxnNum];
	      newEmitted[decayRxnNum++] = emitted[rxnNum];
	    }
	  else
	    {
	      delete paths[rxnNum];
	      delete emitted[rxnNum];
	    }

      /* copy total decay rate */
      newPaths[decayRxnNum] = paths[rxnNum];

      delete relations;
      delete paths;
      delete emitted;
      relations = newDaug;
      paths = newPaths;
      emitted = newEmitted;
    }


  nPaths = numDecay;
  
  if (nPaths == 0)
    return TRUNCATE;
  else
    return TRUNCATE_STABLE;
  
}
      
