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
int NuclearData::mode = MODE_FORWARD;

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
  
  /* set dimension */
  nPaths = n.nPaths;

  /* initialize all pointers to NULL */
  relations=NULL;
  emitted=NULL;
  single=NULL;
  paths=NULL;
  P=NULL;
  D=NULL;
  for (int dHeat=0;dHeat<3;dHeat++)
    E[dHeat] = 0;

  if (nPaths < 0)
    return;

  /* allocate storage */

  /* if there are any paths other than total */
  if (nPaths>0)
    {
      relations = new int[nPaths];
      memCheck(relations,"NuclearData::NuclearData(...) copy constructor: relations");
      
      emitted = new char*[nPaths];
      memCheck(emitted,"NuclearData::NuclearData(...) copy constructor: emitted");
    }
  
  paths = new double*[nPaths+1];
  memCheck(paths,"NuclearData::NuclearData(...) copy constructor: paths");
  
  /* copy data */
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
      
      /* check for pointing of D and P */
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
  
  /* allocate storage for total of paths and single xsection */
  
  paths[nPaths] = new double[nGroups+1];
  memCheck(paths[rxnNum],"NuclearData::NuclearData(...) copy constructor: paths[n]");
  single = new double[nGroups+1];
  memCheck(single,"NuclearData::NuclearData(...) copy constructor: single");
  
  for (gNum=0;gNum<=nGroups;gNum++)
    {
      paths[rxnNum][gNum] = n.paths[rxnNum][gNum];
      single[gNum] = n.single[gNum];
    }
  
  /* check for pointing of D and P */
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


/* basic destructor for NuclearData */
NuclearData::~NuclearData()
{
  cleanUp();
  delete single;
  single = NULL;
  P = NULL;
}

NuclearData& NuclearData::operator=(const NuclearData& n)
{

  if (this == &n)
    return *this;

  int rxnNum,gNum;
 
  cleanUp();
  delete single;
  single = NULL;
  P = NULL;

  nPaths = n.nPaths;

  if (nPaths < 0)
    return *this;

  /* only need relations and emitted if nPaths > 0 */
  if (nPaths>0)
    {

      relations = new int[nPaths];
      memCheck(relations,"NuclearData::NuclearData(...) copy constructor: relations");
      
      emitted = new char*[nPaths];
      memCheck(emitted,"NuclearData::NuclearData(...) copy constructor: emitted");
    }

  /* always need paths */
  paths = new double*[nPaths+1];
  memCheck(paths,"NuclearData::NuclearData(...) copy constructor: paths");
  
  /* copy data */
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

  /* allocate and fill total xsections */
  paths[nPaths] = new double[nGroups+1];
  memCheck(paths[nPaths],"NuclearData::NuclearData(...) copy constructor: paths[n]");
  single = new double[nGroups+1];
  memCheck(single,"NuclearData::NuclearData(...) copy constructor: single");
  
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

  return *this;


}  


void NuclearData::cleanUp()
{ 
  int rxnNum;

  if (nPaths>=0)
    {
      for (rxnNum=0;rxnNum<nPaths;rxnNum++)
	{
	  delete paths[rxnNum];
	  delete emitted[rxnNum];
	}
      delete paths[rxnNum];
    }
  delete paths;
  delete emitted;
  delete relations;

  paths = NULL;
  emitted = NULL;
  relations = NULL;
  D = NULL;

  E[0] = 0;
  E[1] = 0;
  E[2] = 0;

  nPaths = -1;

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
			  char** emissions, float** xSection, float thalf, float *totalXSection)
{
  int gNum, rxnNum;

  verbose(4,"Setting NuclearData members.");

  cleanUp();

  /* set dimensions */
  nPaths = numRxns;

  if (nPaths < 0)
    return;

  /* only need relations and emitted if nPaths > 0 */
  if (nPaths > 0)
    {
      relations = new int[nPaths];
      memCheck(relations,"NuclearData::setData(...) : relations");
      
      emitted = new char*[nPaths];
      memCheck(emitted,"NuclearData::setData(...) : emitted");
    }

  paths = new double*[nPaths+1];
  memCheck(paths,"NuclearData::setData(...) : paths");

  E[0] = radE[0];  
  E[1] = radE[1];  
  E[2] = radE[2];

  /* initialize total x-sections */

  /* total of all paths */
  paths[nPaths] = new double[nGroups+1];
  memCheck(paths[nPaths],"NuclearData::setData(...) : paths[n]");
  for (gNum=0;gNum<nGroups;gNum++)
    paths[nPaths][gNum] = 0;

  /* if we are passed a total xsection (we must be in reverse mode) */
  if (totalXSection != NULL)
    {  
      delete single;
      single = NULL;
      P = NULL;

      single = new double[nGroups+1];
      for (gNum=0;gNum<nGroups;gNum++)
	single[gNum] = totalXSection[gNum]*1e-24;
      D = single;
    }
  else
    D = paths[nPaths];

  if (thalf>0)
    D[nGroups] = log(2.0)/thalf;
  else
    D[nGroups] = 0;

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
  int *newDaug = NULL;
  char **newEmitted = NULL;
  /* count decay reactions */
  int numDecay = 0;
  for (rxnNum=0;rxnNum<nPaths;rxnNum++)
    if (paths[rxnNum][nGroups]>0)
      numDecay++;

  /* if there are fewer decay reactions than the total number, 
   * copy them to a new array */
  if (numDecay < nPaths)
    {
      /* only need relations and emitted if we have decay reactions */
      if (numDecay > 0)
	{
	  newDaug = new int[numDecay];
	  memCheck(newDaug,"NuclearData::stripNonDecay(...): newDaug");
	  
	  newEmitted = new char*[numDecay];
	  memCheck(newEmitted,"NuclearData::stripNonDecay(...): newEmitted");
	}  

      int decayRxnNum = 0;

      /* always need at least one path array for the total */
      double **newPaths = new double*[numDecay+1];
      memCheck(newPaths,"NuclearData::stripNonDecay(...): newPaths");

      /* always need to copy the decay paths (could be none) and
       * delete the non decay paths */
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
      
      /* always must copy total decay rate */
      newPaths[decayRxnNum] = paths[rxnNum];

      delete relations;
      delete emitted;
      delete paths;
      relations = newDaug;
      emitted = newEmitted;
      paths = newPaths;
      
      nPaths = numDecay;
    }
  

  if (nPaths == 0)
    return TRUNCATE;
  else
    return TRUNCATE_STABLE;
  
}
      
