/* $Id: NuclearData.C,v 1.18 2007-04-19 19:38:15 phruksar Exp $ */
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

/** The default constructor initializes NuclearData::nPaths to -1,
    for use later, sets all the pointers to NULL, and zeroes the
    NuclearData::E[] array. */
NuclearData::NuclearData()
{
  nPaths=-1;
  origNPaths=nPaths;
  relations=NULL;
  emitted=NULL;
  single=NULL;
  paths=NULL;
  P=NULL;
  D=NULL;
  for (int dHeat=0;dHeat<3;dHeat++)
    E[dHeat] = 0;
}

/** The copy constructor copies all the data on an
    element-by-element basis, allocating new storage where necessary.
    It does \b NOT simply copy pointers. */
NuclearData::NuclearData(const NuclearData& n)
{

  int rxnNum,gNum;
  
  /* set dimension */
  nPaths = n.nPaths;
  origNPaths = n.origNPaths;

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
  E[2] = n.E[2];
  

}  

NuclearData::~NuclearData()
{
  cleanUp();
  delete[] single;
  single = NULL;
  P = NULL;
}

/** The assignment operator copies all the data on an
    element-by-element basis, freeing old storage and allocating new
    storage where necessary.  It does \b NOT simply copy pointers. */
NuclearData& NuclearData::operator=(const NuclearData& n)
{

  if (this == &n)
    return *this;

  int rxnNum,gNum;
 
  cleanUp();
  delete[] single;
  single = NULL;
  P = NULL;

  nPaths = n.nPaths;
  origNPaths = n.origNPaths;

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
  E[2] = n.E[2];

  return *this;


}  


/** This task might be required in a variety of places so the
    functionality was encapsulated into a single function. */
void NuclearData::cleanUp()
{ 
  int rxnNum;

  if (nPaths>=0)
    {
      for (rxnNum=0;rxnNum<nPaths;rxnNum++)
    {
      delete[] paths[rxnNum];
      delete[] emitted[rxnNum];
    }
      delete[] paths[rxnNum];
    }
  delete[] paths;
  delete[] emitted;
  delete[] relations;

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

/** This function reads the library type from the input file attached
    to the stream reference argument and calls for the creation of a
    new DataLib object through the DataLib::newLib(...) function.  It
    also requests the number of groups from the dataLib to set its own
    static member 'nGroups' and share the info with other classes
    (e.g. VolFlux). */
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
/** This function implements a callback from the nuclear data
    library modules.  The arguments for this function are the data as
    read from the library (Note that single precision is sufficient
    for library data).  These data are copied into
    NuclearData::nPaths, NuclearData::E, NuclearData::relations,
    NuclearData::emitted, NuclearData::paths, NuclearData::D[ngroups],
    and NuclearData::single respectively. */
void NuclearData::setData(int numRxns, float* radE, int* daugKza, 
			  char** emissions, float** xSection, 
			  float thalf, float *totalXSection)
{
  int gNum, rxnNum, totalRxnNum=-1;
  
  verbose(4,"Setting NuclearData members.");

  cleanUp();

  /* set dimensions */
  nPaths = numRxns;
  origNPaths = nPaths;

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
  if ( (NuclearData::mode == MODE_REVERSE) && (totalXSection != NULL) )
    {  
      delete[] single;
      single = NULL;
      P = NULL;

      single = new double[nGroups+1];
      for (gNum=0;gNum<nGroups;gNum++)
	single[gNum] = totalXSection[gNum]*1e-24;
      D = single;
    }
  else
    D = paths[nPaths];

  //For FEINDLib, a value of thalf is "inf" for a stable isotope. D[nGroups] will be re-initialized. 
  if (thalf>0)
    D[nGroups] = log(2.0)/thalf;
  else
    D[nGroups] = 0;

  /* setup each reaction */
  if ( (NuclearData::mode == MODE_REVERSE) || (totalXSection == NULL) ) // ALARALib and ADJLib always yield "true"
    for (rxnNum=0;rxnNum<nPaths;rxnNum++)
      {
        debug(4,"Copying reaction %d with %d groups.",rxnNum,nGroups+1);
        relations[rxnNum] = daugKza[rxnNum];
        /* log location of total reaction */
        if (daugKza[rxnNum] == 0)
	  totalRxnNum = rxnNum;
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
  else //FIENDLib always comes here. It doesn't have a emitted particle implemented
    {
    for (rxnNum=0;rxnNum<nPaths;rxnNum++)
      {
        debug(4,"Copying reaction %d with %d groups.",rxnNum,nGroups+1);
        relations[rxnNum] = daugKza[rxnNum];
        /* log location of total reaction */
        if (daugKza[rxnNum] == 0)
	  totalRxnNum = rxnNum;
        emitted[rxnNum] = new char[strlen(emissions[rxnNum])+1];
        memCheck(emitted[rxnNum],"NuclearData::setData(...) : emitted[n]");
        strcpy(emitted[rxnNum],emissions[rxnNum]);

        paths[rxnNum] = new double[nGroups+1];
        memCheck(paths[rxnNum],"NuclearData::setData(...) : paths[n]");

        for (gNum=0;gNum<nGroups;gNum++)
	  {
	    paths[rxnNum][gNum] = xSection[rxnNum][gNum]*1e-24;
// 	    if (strcmp(emitted[rxnNum],"x"))
// 	      paths[nPaths][gNum] += paths[rxnNum][gNum];
	  }
        paths[rxnNum][nGroups] = xSection[rxnNum][nGroups];
      }

      //Initialize path[nPaths] with totalXSection
       for (gNum=0; gNum<nGroups;gNum++)
         paths[nPaths][gNum] = totalXSection[gNum]*1e-24;

       paths[nPaths][nGroups] = totalXSection[nGroups];
    }

  if (totalRxnNum>-1)
    {
      /* make sure that the decay constant is copied */
      paths[totalRxnNum][nGroups] = paths[nPaths][nGroups];

      /* check where D is pointing */
      if (D == paths[nPaths])
	D = paths[totalRxnNum];

      /* delete summation based total reaction rate */
      delete[] paths[nPaths];

      /* point nPaths to the totalRxnNum */
      paths[nPaths] = paths[totalRxnNum];

      /* delete the emitted entry for the total reaction rate */
      delete[] emitted[totalRxnNum];

      /* shift all channel reactions down by one */
      for (rxnNum=totalRxnNum;rxnNum<nPaths-1;rxnNum++)
	{
	  paths[rxnNum] = paths[rxnNum+1];
	  relations[rxnNum] = relations[rxnNum+1];
	  emitted[rxnNum] = emitted[rxnNum+1];
	}
      /* unpoint the last emitted */
      emitted[nPaths-1] = NULL;
      
      /* shift the total destruction rate down */
      nPaths--;
      paths[nPaths]=paths[nPaths+1];
      paths[nPaths+1] = NULL;
      
    }

  origNPaths = nPaths;

}

/** This change was necessary to enable the RateCache concept to
    work since it is necessary to have the reactions indexed the
    same way, even if non-decay reactions have been stripped from
    the list.  After sorting, decay reaction 1...d will always be
    1...d whether or not the non-decay reactions are present. */
void NuclearData::sortData()
{
  int rxnNum=0,switchNum=nPaths;
  int tmpRel;
  char *tmpEmit;
  double *tmpPath;

  if ( paths[nPaths][nGroups] > 0)
    while (rxnNum < switchNum)
      {
	/* find last decay path */
	while (paths[--switchNum][nGroups]==0 && switchNum > 0) ;
	
	/* find first non-decay path */
	while (rxnNum < switchNum && paths[rxnNum][nGroups]>0) rxnNum++;
	
	if (rxnNum < switchNum)
	  {
	    tmpPath = paths[rxnNum];
	    tmpRel = relations[rxnNum];
	    tmpEmit = emitted[rxnNum];
	    paths[rxnNum] = paths[switchNum];
	    relations[rxnNum] = relations[switchNum];
	    emitted[rxnNum] = emitted[switchNum];
	    paths[switchNum] = tmpPath;
	    relations[switchNum] = tmpRel;
	    emitted[switchNum] = tmpEmit;
	  }
      }
}
				   

/** Both the NuclearData::nPaths member and the related arrays
    (NuclearData::paths, NuclearData::emitted, and NuclearData::relation)
    are reduced to reflect the new number of reaction paths.  This is
    used when the truncation state has determined that at most
    radioactive branches should be considered in the chain creation
    process.  After counting the radioactive decay branches, it
    returns an adjusted truncation state. */
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
	    delete[] paths[rxnNum];
	    delete[] emitted[rxnNum];
	  }
      
      /* always must copy total decay rate */
      newPaths[decayRxnNum] = paths[rxnNum];

      delete[] relations;
      delete[] emitted;
      delete[] paths;
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
      
