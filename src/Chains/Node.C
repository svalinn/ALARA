/* $Id: Node.C,v 1.28 2007-02-28 23:25:37 phruksar Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Chain: functions directly related to the building and analysis of chains
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "Node.h"

#include "DataLib/DataLib.h"

#include "truncate.h"

#include "Util/Statistics.h"

#include "Calc/VolFlux.h"

/****************************
 ********* Service **********
 ***************************/

DataCache Node::lambdaCache;
DataCache Node::heatCache;
DataCache Node::alphaCache;
DataCache Node::betaCache;
DataCache Node::gammaCache;
DataCache Node::wdrCache;

/** When called with no arguments this sets the KZA value to 0.
      Otherwise, it processes the isotope name passed as the
          argument and converts it to a KZA number.  Default
          constructors are used for the base class. */
Node::Node(char *isoName)
{

  kza = 0;
  if (isoName != NULL)
    {
      debug(4,"Making new Node: %s.",isoName);
      char cpyName[8], sym[5];
      strcpy(cpyName,isoName);
      
      char *strPtr = strchr(cpyName,'-');
      
      int A = atoi(strPtr+1);
      *strPtr = '\0';
      
      sprintf(sym," %s ",cpyName);
      
      debug(5,"Set A= %d and searching for symbol '%s'",A,sym);

      int Z = (strstr(SYMBOLS,sym)-SYMBOLS)/3 + 1;
      
      kza = (Z*1000+A)*10;
    }
}

/** It passes arguments 2,4, and 5 to TreeInfo, argument 3
    to NuclearData and initializing 'kza' with argument 1. */

Node::Node(int nextKza, Node* passedPrev, double* passedSingle, 
	   int passedRank, int passedState) 
  : TreeInfo(passedPrev,passedRank,passedState), 
    kza(nextKza)
{ 

  int gNum;

  switch(mode)
    {
    case MODE_FORWARD:
      single = new double[nGroups+1];
      memCheck(single,"Node::Node(...) constructor: single");
      for (gNum=0;gNum<=nGroups;gNum++)
	single[gNum] = passedSingle[gNum];
      P = single;
      break;
    case MODE_REVERSE:
      prev->P = passedSingle;
      break;
    }

}



/****************************
 ********** Chain ***********
 ***************************/

/* function to read nuclear data from arbitrary data source */
/* called by Chain::build() */
/** NOTE: DataLib::readData() is a virtual function,
      implemented for each specific type of data library.  The 'this'
      pointer is passed to give a callback object for setting the data
      once it has been read from the file
      (see NuclearData::setData(...)). */
void Node::readData()
{
  dataLib->readData(kza,this);

  sortData();

  switch(mode)
    {
    case MODE_FORWARD:
      /* this is only true for forward mode */
      /* if a new node is created with TRUNCATE_STABLE state
       * strip its pure transmutation reactions immediately */
      if (state == TRUNCATE_STABLE)
	state = stripNonDecay();
      break;
    case MODE_REVERSE:
      if (single != NULL)
	D = single;
      else
	D = prev->P;
      break;
    }
}


double Node::getLambda(int setKza)
{
  if (!lambdaCache.count(setKza))
    {
      kza = setKza;
      readData();
      
      if (nPaths>0 && D[nGroups]>0)
	lambdaCache[kza] = D[nGroups];
      else
	lambdaCache[kza] = 0;
    }

  return lambdaCache[setKza];
}

double Node::getHeat(int setKza)
{
  if (!heatCache.count(setKza))
    {
      kza = setKza;
      readData();
      
      if (nPaths>0 && D[nGroups]>0)
	heatCache[kza] = D[nGroups] * (E[0]+E[1]+E[2]);
      else
	heatCache[kza] = 0;
    }

  return heatCache[setKza];

}

double Node::getAlpha(int setKza)
{
  if (!alphaCache.count(setKza))
    {
      kza = setKza;
      readData();
      
      if (nPaths>0 && D[nGroups]>0)
	alphaCache[kza] = D[nGroups] * E[2];
      else
	alphaCache[kza] = 0;
    }
  
  return alphaCache[setKza];
    
}

double Node::getBeta(int setKza)
{
  if (!betaCache.count(setKza))
    {
      kza = setKza;
      readData();
      
      if (nPaths>0 && D[nGroups]>0)
	betaCache[kza] = D[nGroups] * E[0];
      else
	betaCache[kza] = 0;
    }
  return betaCache[setKza];
}

double Node::getGamma(int setKza)
{
  if (!gammaCache.count(setKza))
    {
      kza = setKza;
      readData();
      
      if (nPaths>0 && D[nGroups]>0)
	gammaCache[kza] = D[nGroups] * E[1];
      else
	gammaCache[kza] = 0;
    }

  return gammaCache[setKza];

}

double Node::getWDR(int setKza)
{
  if (wdrCache.count(setKza))
    return wdrCache[setKza];
  else
    return 0;
}

void Node::loadWDR(char *fname)
{

  int tmpKza, A, Z, mode;
  char isoName[16], sym[5], *strPtr, isoFlag;
  double wdr;
  Node dataAccess;
  ifstream wdrFile(searchNonXSPath(fname),ios::in);

  wdrCache.erase(wdrCache.begin(),wdrCache.end());

  wdrFile >> isoName >> wdr;

  if (isdigit(isoName[0]))
    mode = WDR_KZAMODE;
  else
    mode = WDR_SYMMODE;

  while (!wdrFile.eof())
    {
      if (mode == WDR_SYMMODE)
	{
	  for (strPtr=isoName;*strPtr;strPtr++)
	    *strPtr = tolower(*strPtr);
	  strPtr = strchr(isoName,'-');
	  *strPtr = '\0';
	  
	  sprintf(sym," %s ",isoName);
	  
	  Z = (strstr(SYMBOLS,sym)-SYMBOLS)/3 + 1;
	  isoFlag = '\0';
	  sscanf(strPtr+1,"%d%c",&A,&isoFlag);
	  tmpKza = (Z*1000+A)*10;
	  if (isalpha(isoFlag))
	    tmpKza += isoFlag - 'l';
	}
      else
	tmpKza = atoi(isoName);

      wdrCache[tmpKza] = dataAccess.getLambda(tmpKza)/wdr;
      wdrFile >> isoName >> wdr;
    }

  wdrFile.close();

}

/* function to copy pointers to the chain data */
/* called by Chain::build() */
/** It also sets the appropriate entry of
      'Chain::loopRank[]' passed in the last argument.  The second
      argument indicates the size of the arrays. */
void Node::copyRates(double **rates, const int step, int *loopRank)
{


  if (D == NULL)
    {
      rates[rank+step] = NULL;
      rates[rank+3*step] = NULL;
    }
  else
    {
      rates[rank+step] = D;
      rates[rank+3*step] = rates[rank+step]+nGroups;
    }

  /* in reverse mode, we have to set the daughter's P rate
   * and not this parent's */
  int idx = rank;
  double *setP = P;
  if (mode == MODE_REVERSE && idx > 0)
    {
      idx = rank-1;
      setP = prev->P;
    }

  if (setP == NULL)
    {
      rates[idx] = NULL;
      rates[idx+2*step] = NULL;
    }
  else
    {
      rates[idx] = setP;
      rates[idx+2*step] = rates[idx]+nGroups;
    }

  /* find loop rank and assume that this node has no loop */
  int foundLoop = findLoop();
  loopRank[rank] = -1;

  /* find natural loop with current isotope */
  switch(mode)
    {
    case MODE_FORWARD:
      {
	/* if there is a natural loop */
	if (foundLoop>-1)
	  loopRank[rank] = rank - foundLoop;

	/* check for internal loop */
	if (rank > 0 && loopRank[rank-1] > -1 
	    && (loopRank[rank] > loopRank[rank-1]+1 || loopRank[rank] == -1))
	  loopRank[rank] = loopRank[rank-1] + 1;
	
	break;
      }
    case MODE_REVERSE:
      {

	/* if there is a natural loop */
	if (foundLoop>-1)
	  {
	    loopRank[foundLoop] = rank - foundLoop;
	    
	    /* check for internal loop */
	    if (loopRank[foundLoop] > loopRank[foundLoop+1]+1)
	      loopRank[foundLoop] = loopRank[foundLoop+1]+1;
	  }

	/* propagate that loop to other chain members */
	while (--foundLoop >= 0)
	  if (loopRank[foundLoop] == -1)
	    loopRank[foundLoop] = loopRank[foundLoop+1]+1;

	break;
      }
    }

	  

}

/* re-initialize elements when the chain retracts */
/** It does this before retracting the chain so that they are at the
    correct default before advancing the chain again. */
void Node::delRates(double **rates, const int step, int *loopRank)
{
  int idx;

  rates[rank] = NULL;
  rates[rank+step] = NULL;
  rates[rank+2*step] = NULL;
  rates[rank+3*step] = NULL;
  switch(mode)
    {
    case MODE_FORWARD:
      /* simply reset this value */
      loopRank[rank] = -1;
      break;
    case MODE_REVERSE:
      /* reset this value */
      loopRank[rank] = -1;
      idx = rank;
      while (--idx >= 0)
	if (loopRank[idx]+idx >= rank)
	  loopRank[idx] = -1;
      break;
    }
}


/** function to determine state of tree building process.
    This function is called twice from Chain::build(...).  The first
    time is only for new isotopes which, by logic, must be in state
    CONTINUE or TRUNCATE_STABLE.  This is when the argument is used.
    The subsequent call is used to determine the action of the
    Chain::build(...) function.  The arguments defaults to 0 and the
    state upon entering the function could be one of many.
    called by Chain::build() */
int Node::stateEngine(int stateBits)
{

  /* if we are doing an initial truncation state analysis */
  if (stateBits>-1)
    {
      switch(state)
	{
	case CONTINUE:
	  if (stateBits == IGNORE)
	    state = IGNORE;
	  else if (stateBits >= TRUNCATE_STABLE && mode == MODE_FORWARD)
	    state = stripNonDecay();
	  else if (nPaths == 0 || stateBits >= TRUNCATE)
	    state = TRUNCATE;
	  break;
	case TRUNCATE_STABLE:
	  if (stateBits == IGNORE)
	    state = IGNORE;
	  else 
	    /* NOTE: this test is easier than case CONTINUE since:
	     * 1) stripNonDecay() is run during readData()
	     * 2) since this is a next of something in a form
	     *    of TRUNCATE state, this must also be in a form of 
	     *    TRUNCATE state
	     * 3) there is no change of state if it is radioactive.
	     */
	    if (nPaths == 0 || stateBits > TRUNCATE)
	      state = TRUNCATE;
	  break;
	}
    }
  else
    /* chain building action analysis */
    switch(state)
      {
      case IGNORE:
	/* When we are ignoring the last next, we must change the
	 * prev's state back to truncate so that its chain will not be
	 * lost. */
	if (prev != NULL && prev->state == SOLVED)
	  prev->state = TRUNCATE;
	break;
      case TRUNCATE:
	/* During the initial call to the state engine for a new isotope
	 * the state can be set to TRUNCATE.  The state engine then starts
	 * with this state when determining the action for Chain::build(...).
	 * The appropriate action is to solve the chain.
	 */
	state = SOLVE;
	break;
      case SOLVE:
	/* This state is set when a chain is truncated.   
	 *
	 * The appropriate action here is to initiate a retraction of the chain.
	 */
	state = SOLVED;
	/* if we just solved the root, set the terminal state */
	if (rank == 0)
	  state = FINISHED_ROOT;
	break;
      case SOLVED:
	/* if we just solved the root, set the terminal state */
	if (rank == 0)
	  state = FINISHED_ROOT;
	break;
      }
  
  return state;

}

/* function to add correct next product to chain */
/* called by Chain::build() */
/** It uses the information stored in the Node object through
      which it is called to initialize a newly created Node object.
          The argument is assigned a value to indicate which rank should
          be tallied if/when this chain is sovled. A pointer to this new
          object is returned. */
Node* Node::addNext(int &setRank)
{

  Node *ptr = this;

  /* If there are more children, add them.  */
  if (pathNum < nPaths)
    {
      next = new Node(relations[pathNum],this,paths[pathNum],rank+1,state);
      memCheck(next,"Node::addNext(): next");
    }
  else
    /* NOTE: this function should never be called in a state where
     * above is not true.  */
    error(9000,"Programming error: trying to add too many relations in chain.");

  /* If this was the last next */
  if (++pathNum == nPaths)
    {
      /* This node will be solved and tallied with its last next */
      state = SOLVED;

      /* look back up chain for successive isotopes which are also on
       * their last next */
      while (ptr->prev != NULL && ptr->prev->state == SOLVED)
	ptr = ptr->prev;

      /* setRank indicates that all nodes after this one (ptr) should be
       * tallied at the next solution */
      setRank = ptr->rank;
    }
  else
    /* if not the last next, only the last/newest node should be
     * tallied */
    setRank = next->rank;

  /* return the newly created next node */
  return next;
}

void Node::prune()
{ 
  delete next; 
  next = NULL; 
};

/** Retract the chain by returning the prev - usually followed by a 
    call to prune() */
Node* Node::retract()
{
  return prev;
}

/** The rank of this isotope is returned for use in Chain::loopRank[].*/
int Node::findLoop()
{
  Node* nodePtr = prev;

  while (nodePtr != NULL)
    {
      if (nodePtr->kza == kza)
	return nodePtr->rank;
      nodePtr = nodePtr->prev;
    }

  return -1;
}
  
/** If the rank is greater than the chainlength, '-1' is returned. */
int Node::getRankKza(int findRank)
{
  Node* ptr = this;

  while (findRank>ptr->rank && ptr->next != NULL)
    ptr = ptr->next;
  
  if (findRank == ptr->rank)
    return ptr->kza;
  else
    return -1;
}

/** The reaction rate vector pointed to by the
      first argument is used to determine whether the current reaction
      is a destruction rate or a production rate.  In the former case,
      the current node's kza value is used for the base isotope, while
      in the latter, the parent's kza value is used for the base
      isotope.  Based on this choice, the last three arguments are
      assigned values for the base kza, the reaction number index (0
      for a destruction rate), and the original number of reaction paths
      for this base isotope.  This function is called by VolFlux::fold()
      for use in cache processing. */
void Node::getRxnInfo(double *rateVec, int &baseKza, int &rxnNum, int &numRxns)
{

  Node *base;

  switch(mode)
    {
    case MODE_FORWARD:
      base = prev;
      break;
    case MODE_REVERSE:
      base = this;
      break;
    }
  
  baseKza = -1;
  
  /* if this is a production rate */
  if (rateVec == P)
    {
      if (P != NULL)
	{
	  baseKza = base->kza;
	  rxnNum = base->pathNum;
	  numRxns = base->origNPaths;
	}
    }
  /* otherwise this is a destruction rate */
  else
    {
      baseKza = kza;
      rxnNum = 0;
      numRxns = origNPaths;
    }
      
}

/** This function retrieves the nx cross sections, and returns the
    values in a matrix that has dimensions numCP x nGroups */
double** Node::getCPXS(int findKZA)
{
  // Get data from VolFlux Class
  int nCP = VolFlux::getNumCP();

  kza = findKZA;
  readData();
  
  int cpKZA[5] = { 20040,
		   10020,
		   20030,
		   10010,
		   10030 };

  // Alpha, Deuteron, Helium3, Proton, Triton

  double **CPXS = new double*[nCP];
  double *CPXSStorage = new double[nGroups*nCP];
  
  for(int i = 0; i < nCP; i++)
    {
      CPXS[i] = &CPXSStorage[i*nGroups];
      for(int j = 0; j < nGroups; j++)
	{
	  CPXS[i][j] = 0;
	}
    }
  
  // Look for appropriate entry
  for(int i = 0; i < nPaths; i++)
    {
      if(!strcmp(emitted[i],"x"))
	for(int j = 0; j < nCP; j++)
	  {
	    if(relations[i] == cpKZA[j])
	      for(int k = 0; k < nGroups; k++)
		{
		  CPXS[j][k] = paths[i][k];
		}
	  }
    }

  return CPXS;
}
