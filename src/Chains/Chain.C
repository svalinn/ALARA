/* $Id: Chain.C,v 1.25 2007-12-11 21:33:55 phruksar Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Chain: functions directly related to the building and analysis of chains
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "Chain.h"
#include "Root.h"

#include "Input/Volume.h"

#include "Calc/VolFlux.h"
#include "Calc/topSchedule.h"
#include "Calc/topScheduleT.h"

#include "truncate.h"
#include "Util/Matrix.h"

/****************************
 ********* Service **********
 ***************************/
double Chain::truncLimit = 1;
double Chain::ignoreLimit = 1e-2;
double Chain::impurityDefn = 0;
double Chain::impurityTruncLimit = 1;
int Chain::mode = MODE_FORWARD;


/** Establishes a chain with 'maxChainLength' equal to the
    default, INITMAXCHAINLENGTH, and the corresponding storage
    for 'loopRank' and 'rates'.  When called with no arguments,
    all other members are set to 0 or NULL.  Otherwise, the
    'root' and 'node' pointers are set to the first argument. */
Chain::Chain(Root *newRoot, topSchedule *top)
{
  int rank;

  maxChainLength = INITMAXCHAINLENGTH;

  chainLength = 0;
  newRank = 0;
  setRank = 0;
  solvingRef = FALSE;

  loopRank = NULL;
  rates = NULL;
  
  loopRank = new int[maxChainLength];
  memCheck(loopRank,"Chain::Chain(...) constructor: loopRank");

  rates = new double*[6*maxChainLength];
  memCheck(rates,"Chain::Chain(...) constructor: rates");

  for (rank=0;rank<maxChainLength;rank++)
    {
      for (int set=0;set<6;set++)
	rates[set*maxChainLength + rank] = NULL;
      loopRank[rank] = -1;
    }

  colRates = NULL;

  root = newRoot;
  node = root;

  chainTruncLimit = truncLimit;
  chainIgnoreLimit = truncLimit*ignoreLimit;

  verbose(2,"   Maximum relative concentration: %g",root->maxConc());

  if (root->maxConc() < impurityDefn && mode == MODE_FORWARD)
    {
      chainTruncLimit = impurityTruncLimit;
      chainIgnoreLimit = impurityTruncLimit*ignoreLimit;
      verbose(2,"   treating as impurity");
    }

  reference = NULL;
  if (newRoot != NULL && top != NULL)
    {
      reference = new Volume(newRoot,top);
      memCheck(reference,"Chain::Chain(...) constructor: reference");
    }

  debug(3,"Created new chain.");
}

/** Copies all the scalar members directly and copies the vectors
    on an element-by-element basis.  The sizes of the vectors are
    (obviously?) based on the copied 'maxChainLength' member. */

Chain::Chain(const Chain& c)
{
  int rank,sliceSize;

  maxChainLength = c.maxChainLength;

  chainLength = c.chainLength;
  newRank = c.newRank;
  setRank = c.setRank;
  solvingRef = c.solvingRef;
  
  loopRank = NULL;
  rates = NULL;

  loopRank = new int[maxChainLength];
  memCheck(loopRank,"Chain::Chain(...) copy constructor: loopRank");
  for (rank=0;rank<maxChainLength;rank++)
    loopRank[rank] = c.loopRank[rank];

  rates = new double*[6*maxChainLength];
  memCheck(rates,"Chain::Chain(...) copy constructor: rates");
  for (rank=0;rank<6*maxChainLength;rank++)
    rates[rank] = c.rates[rank];
  
  colRates = NULL;
  sliceSize = VolFlux::getNumFluxes()*chainLength;
  if (sliceSize > 0)
    {
      colRates = new double[4*sliceSize];
      memCheck(colRates,"Chain::Chain(...) copy constructor: colRates");
      for (rank=0;rank<4*sliceSize;rank++)
	colRates[rank] = c.colRates[rank];
    }

  root = c.root;
  node = c.node;

  chainTruncLimit = c.chainTruncLimit;
  chainIgnoreLimit = c.chainIgnoreLimit;

  reference = c.reference;
}

Chain::~Chain()
{ 
  delete loopRank;
  delete rates; 
  delete colRates; 
  delete reference;
  root->cleanUp(); 
};

/** The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store
    before allocating new space into which to copy the object. */
Chain& Chain::operator=(const Chain& c)
{

  if (this == &c)
    return *this;

  int rank,sliceSize;

  maxChainLength = c.maxChainLength;

  chainLength = c.chainLength;
  newRank = c.newRank;
  setRank = c.setRank;
  solvingRef = c.solvingRef;

  delete loopRank;
  delete rates;
  loopRank = NULL;
  rates = NULL;

  loopRank = new int[maxChainLength];
  memCheck(loopRank,"Chain::Chain(...) copy constructor: loopRank");
  for (rank=0;rank<maxChainLength;rank++)
    loopRank[rank] = c.loopRank[rank];

  rates = new double*[6*maxChainLength];
  memCheck(rates,"Chain::Chain(...) copy constructor: rates");
  for (rank=0;rank<6*maxChainLength;rank++)
    rates[rank] = c.rates[rank];
  
  delete colRates;
  colRates = NULL;
  sliceSize = VolFlux::getNumFluxes()*chainLength;
  if (sliceSize > 0)
    {
      colRates = new double[4*sliceSize];
      memCheck(colRates,"Chain::Chain(...) copy constructor: colRates");
      for (rank=0;rank<4*sliceSize;rank++)
	colRates[rank] = c.colRates[rank];
    }

  root = c.root;
  node = c.node;

  chainTruncLimit = c.chainTruncLimit;
  chainIgnoreLimit = c.chainIgnoreLimit;

  reference = c.reference;

  return *this;

}

/****************************
 ********** Input ***********
 ***************************/

void Chain::getTruncInfo(istream& input)
{
  input >> truncLimit;

  verbose(2,"Truncation parameters set at %g for truncation and %g for ignore.",
	  truncLimit, truncLimit*ignoreLimit);
}

void Chain::getIgnoreInfo(istream& input)
{
  input >> ignoreLimit;

  verbose(2,"Truncation parameters set at %g for truncation and %g for ignore.",
	  truncLimit, truncLimit*ignoreLimit);
  if (impurityDefn>0)
    verbose(2,"Impurity defined as %g with truncation parameters set at %g for truncation and %g for ignore.",
	    impurityDefn, impurityTruncLimit, impurityTruncLimit*ignoreLimit);
}

void Chain::getImpTruncInfo(istream& input)
{
  input >> impurityDefn >> impurityTruncLimit;

  verbose(2,"Impurity defined as %g with truncation parameters set at %g for truncation and %g for ignore.",
	  impurityDefn, impurityTruncLimit, impurityTruncLimit*ignoreLimit);
}

/****************************
 ********** Chain ***********
 ***************************/

/* function to perform main truncation algorithm with
 * possible outcomes:
 * CONTINUE
 * DECAY
 * IGNORE
 */
/* If the main truncation state is DECAY this function
 * should remove all pure transmutation reactions */
/* called by Chain::build(...) */
/** It has the primary responsibility for setting the initial
    truncation state. */
void Chain::setState(topSchedule* top)
{
  double *relProd = NULL;
  int coolNum;
  int nCoolingTimes = topScheduleT::getNumCoolingTimes();
  int truncBits=0;

  /* TRUNCATE state may be set in readData(...) after all the
   * reactions have been removed
   */
  if (node->stateEngine(truncBits) == TRUNCATE)
    {
      verbose(3,"Checking node %d",node->count(relProd));
      return;
    }

  solvingRef = TRUE;

  verbose(4,"Running truncation reference calculation.");
  
  /* set decay matrices */
  chainCode++;
  setupColRates();
  top->setDecay(this);
  
  /* perform reference calculation */
  relProd = (reference->solveRef(this,top))->results(chainLength-1);
  
  solvingRef = FALSE;
  
  /* establish bit field */
  truncBits |= TRUNCEOS*(relProd[0]<chainTruncLimit);
  truncBits |= IGNOREOS*(relProd[0]<chainIgnoreLimit);
  
  for (coolNum=0;coolNum<nCoolingTimes;coolNum++)
    {
      truncBits |= TRUNCC*(relProd[coolNum+1]<chainTruncLimit);
      truncBits |= IGNORC*(relProd[coolNum+1]<chainIgnoreLimit);
    }
  
  /* run state engine */
  int state = node->stateEngine(truncBits);
	    
  verbose(3,"Checking node %d",node->count(relProd));

  verbose(4,"Set truncation state: %d (%g)",state,relProd[0]);

  delete relProd;

}

/* function to recursively build chains */
/* called by Root::solve(...) */
/** For each newly added node, it does some initialization and
    sets up the rate pointers.  It then queries the truncation
    state, and acts accordingly.  For example, when continuing
    the chain, it calls on the 'node' to add a daughter, and calls
    itself recursively. */
int Chain::build(topSchedule *top)
{
  /* initialize node */
  if (node->newNode())
    {
      verbose(3,"Processing new node %d at rank %d",node->getKza(), 
	      chainLength);
      chainLength++;
      node->readData();
      /* if our chain is too large, expand the rate vectors */
      resizeRates();
      /* point to new rate vectors and find loopRank*/
      node->copyRates(rates,maxChainLength,loopRank);
      /* set initial truncation state */
      setState(top);
    }
  
  /* depending on current truncation state */
  switch (node->stateEngine())
    {
    case CONTINUE:
    case TRUNCATE_STABLE:
      /* If continuing, add the next daughter and continue the build
       * process.  
       */
      node = node->addNext(setRank);
      return build(top);
      break;
    case SOLVED:
      /* If already solved, then the action is the same as ignoring,
       * except we have to retract newRank as the chain retracts. 
       */
      switch(mode)
	{
	case MODE_FORWARD:
	  newRank = chainLength-1;
	  break;
	case MODE_REVERSE:
	  newRank = 0;
	  break;
	}
    case IGNORE:
      /* If already solved or ignoring, then we are done with this
       * isotope.
       *
       * Retract and prune the chain.
       *
       * Note that newRank does not retract when only ignoring since
       * no solution has been calculated since the last newRank was
       * set.  
       */
      node->delRates(rates,maxChainLength,loopRank);
      node = node->retract();
      node->prune();
      chainLength--;
      return build(top);
      break;
    case SOLVE:
      /* This state is generated by the state engine when truncating.
       * simply return a positive state back through the recursive chain.
       */
      return 1;
      break;
    case FINISHED_ROOT:
      /* terminal condition */
      return 0;
      break;
    }

  error(9000,"Programming Error: Invalid truncation state in Chain::build(...)");
  return -1;
}

/*****************************
 ********* Solution **********
 ****************************/

void Chain::setupColRates()
{

  int rank,idx;

  int sliceSize = VolFlux::getNumFluxes()*chainLength;

  int step = maxChainLength;
  
  delete colRates;
  colRates = NULL;
  if (sliceSize>0)
    {
      colRates = new double[2*sliceSize+2*chainLength];
      memCheck(colRates,"Chain::setupColRates(): colRates");
    }
  else
    error(9000,"Programming Error: Chain::setupColRates(...) \n\
sliceSize = %d [chainLength = %d, VolFlux::getNumFluxes() = %d].",
	  sliceSize, chainLength,VolFlux::getNumFluxes());

  for (rank=0;rank<2*sliceSize+2*chainLength;rank++)
    colRates[rank] = 0;

  P = colRates;
  d = P+sliceSize;
  L = d+sliceSize;
  l = L+chainLength;

  /* set non-flux dependent rates */
  for (rank=0;rank<chainLength;rank++)
    {
      idx = rank;
      if (mode == MODE_REVERSE)
	idx = (chainLength-1)-rank;

      if (rates[rank+2*step] == NULL)
	L[idx] = 0;
      else
	L[idx] = *(rates[rank+2*step]);

      if (rates[rank+3*step] == NULL)
	l[idx] = 0;
      else
	l[idx] = *(rates[rank+3*step]);
    }

  if (solvingRef)
    l[chainLength-1] = 0;

}

/* function to multiply each of the rate vectors of
 * a chain by the list of fluxes for a specfic interval */
void Chain::collapseRates(VolFlux* flux)
{
  int idx,idx2,rank;
  int fluxNum = 0;
  Node *nodePtr;

  int step = maxChainLength;
  
  /* set flux-dependent rates */
  flux = flux->advance();
  while (flux != NULL)
    {
      nodePtr = root;
      for (rank=0;rank<chainLength;rank++)
	{
	  idx = rank;
	  if (mode == MODE_REVERSE)
	    idx = (chainLength-1)-rank;
	  idx2 = fluxNum*chainLength + idx; 
	  P[idx2] = flux->fold(rates[rank],nodePtr)      + L[idx];
	  d[idx2] = flux->fold(rates[rank+step],nodePtr) + l[idx];
	  nodePtr = nodePtr->getNext();
	}
      
      debug(5,"collapsed rates P[last] and d[last]: %12.5e, %12.5e",P[chainLength-1],d[chainLength-1]);

      /* in forward mode, don't destroy bottom isotope */
      if (solvingRef)
	d[fluxNum*chainLength+chainLength-1] = 0;

     fluxNum++;
     flux = flux->advance();
    }

}

/* set the decay matrices, based on chain parameters */
/** It always calls a Bateman method since pure decay cannot have
    loops. This is a member of class Chain to take advantage of the
    chain parameters such as 'chainLength', 'newRank', and 'colRates'.
    This minimizes extra computation by knowing which parts of the
    resultant matrix have already been calcualted and which must be
    newly(re?)-calculated.  Be sure that the matrix referenced in the
    first argument is consistent with this. */
void Chain::setDecay(Matrix& D, double time)
{
  int idx,idx2,row,col, oldSize;
  int localNewRank = newRank;
  int size = chainLength*(chainLength+1)/2;
  int success;
  double *data = new double[size];
  memCheck(data,"Chain::setDecay(...): data");

  /* when solving reference calculations, only the most
   * recent isotope can be a new addition */
  if (mode == MODE_FORWARD && solvingRef)
    localNewRank = std::max(0,chainLength-2);

  oldSize = (localNewRank*(localNewRank+1)/2);

  /* copy previously calculated rows */
  for (idx=0;idx<oldSize;idx++)
    data[idx] = D.data[idx];

  /* fill new rows */
  row = localNewRank;
  col = 0;
  for (;idx<size;idx++)
    {
      if (col == row)
	{
	  data[idx] = exp(-l[row]*time);
	  col = 0;
	  row++;
	}
      else
	{
	  data[idx] = 1;
	  for (idx2=col;idx2<row;idx2++)
	    data[idx] *= L[idx2+1];

	  if (data[idx] > 0)
	    if ( checkForDecayLoop() )
              data[idx] *= laplaceInverse(row, col, l, time, success);
            else
	      data[idx] *= bateman(row,col,l,time,success);

	  col++;
	}
    }

  delete D.data;
  D.data = data;
  D.size = chainLength;

}

/* function to fill a basic transfer matrix */
/* the method used for each element is determined adaptively, 
 * on a element by element basis */
/** It fills the necessary elements of a transfer matrix by adaptively
    choosing the appropriate method: Bateman, Laplace inversion or
    Laplace expansion.  The matrix referenced in the first argument is
    filled using the irradiation time specified in the second argument
    and the flux number specified in the third argument.  *** This is a
    member of class Chain to take advantage of the chain parameters
    such as 'chainLength', 'newRank', and 'colRates'.  This minimizes
    extra computation by knowing which parts of the resultant matrix
    have already been calcualted and which must be
    newly(re?)-calculated.  Be sure that the matrix referenced in the
    first argument is consistent with this. */
void Chain::fillTMat(Matrix& T,double time, int fluxNum)
{
  int idx,row,col,rank,oldSize;
  int localNewRank = newRank;
  int size = chainLength*(chainLength+1)/2;
  int fluxOffset = fluxNum*chainLength;

  double *data = new double[size];
  memCheck(data,"Chain::setDecay(...): data");

  /* when solving reference calculations, only the most
   * recent isotope can be a new addition, BUT
   * the previous isotope needs to have its destruction rates updated
   */
  if (mode == MODE_FORWARD && solvingRef)
    localNewRank = std::max(0,chainLength-2);
  oldSize = (localNewRank*(localNewRank+1)/2);

  /* copy previously calculated rows */
  for (idx=0;idx<oldSize;idx++)
    data[idx] = T.data[idx];
  
  /* fill new rows */
  row = localNewRank;
  rank = row;
  col = 0;
  for (;idx<size;idx++)
    {
      if (col == row)
	{
	  data[idx] = exp(-d[row+fluxOffset]*time);
	  col = 0;
	  row++;
	  switch(mode)
	    {
	    case MODE_FORWARD:
	      rank = row;
	      break;
	    case MODE_REVERSE:
	      rank = (chainLength-1)-row;
	      break;
	    }
	}
      else
	{
	  data[idx] = fillTElement(row,col,P+fluxOffset,d+fluxOffset,time,
				   loopRank,rank);
	  col++;
	}
    }

  /* for reference calculations, don't destroy the last isotope */
  /* if (solvingRef)
    data[size-1] = 1; */

  delete T.data;
  T.data = data;
  T.size = chainLength;

}

/* multiply two matrices carrying saved data */
/** This is a member of class Chain to take advantage of the chain
    parameters such as 'chainLength' and 'newRank'. This minimizes
    extra computation by knowing which parts of the resultant matrix
    have already been calcualted and which must be
    newly(re?)-calculated.  Be sure that the matrix referenced
    in the first argument is consistent with this. */
void Chain::mult(Matrix &result, Matrix& A, Matrix& B)
{
  int idx, idxA, row,col,term, oldSize;
  int localNewRank = newRank;
  int size = chainLength*(chainLength+1)/2;
  double *data = new double[size];
  memCheck(data,"Chain::setDecay(...): data");

  /* when solving reference calculations, only the most
   * recent isotope can be a new addition */
  if (mode == MODE_FORWARD && solvingRef)
    localNewRank = std::max(0,chainLength-2);
  oldSize = (localNewRank*(localNewRank+1)/2);

  /* copy previously calculated rows */
  for (idx=0;idx<oldSize;idx++)
    data[idx] = result.data[idx];
  
  /* fill new rows */
  row=localNewRank;
  col=0;
  idxA=idx;
  for (;idx<size;idx++)
    {
      if (col > row)
	{
	  row++;
	  col=0;
	  idxA = idx;
	}
      data[idx] = 0;
      for (term=col;term<=row;term++)
	data[idx] += A.data[idxA+term]*B.data[term*(term+1)/2+col];
      col++;
    }

  delete result.data;
  result.data = data;
  result.size = chainLength;
}

/****************************
 ********** List ************
 ***************************/

/* function to double the length of the rates array */
void Chain::expandRates()
{
  int rank;

  double **newRates = new double*[6*maxChainLength*2];
  memCheck(newRates,"Chain::expandRates(): newRates");

  int *newloopRank = new int[maxChainLength*2];
  memCheck(newloopRank,"Chain::expandRates(): newloopRank");

  for (rank=0;rank<maxChainLength;rank++)
    {
      for (int set=0;set<6;set++)
	newRates[set*maxChainLength*2 + rank] = 
	  rates[set*maxChainLength + rank];
      newloopRank[rank] = loopRank[rank];
    }
  for (;rank<maxChainLength*2;rank++)
    {
      for (int set=0;set<6;set++)
	newRates[set*maxChainLength*2 + rank] = NULL;
      newloopRank[rank] = -1;
    }
      
  delete rates;
  delete loopRank;
  rates = newRates;
  loopRank = newloopRank;
  maxChainLength *= 2;
}

/* function to double the length of the rates array */
void Chain::compressRates()
{
  double **newRates = new double*[6*maxChainLength/2];
  memCheck(newRates,"Chain::compressRates(): newRates");

  int *newloopRank = new int[maxChainLength/2];
  memCheck(newloopRank,"Chain::expandRates(): newloopRank");

  for (int idx=0;idx<maxChainLength/2;idx++)
    {
      for (int set=0;set<6;set++)
	newRates[set*maxChainLength/2 + idx] = 
	  rates[set*maxChainLength + idx];
      newloopRank[idx] = loopRank[idx];
    }

  delete rates;
  delete loopRank;
  rates = newRates;
  loopRank = newloopRank;
  maxChainLength /= 2;
}

void Chain::resizeRates()
{
  if (chainLength>maxChainLength)
    expandRates();
  else if (maxChainLength>INITMAXCHAINLENGTH &&
	   chainLength<maxChainLength/4)
    compressRates();
}

/****************************
 ********* Utility **********
 ***************************/

/** If the specified rank is greater than the 'chainLength', it will
    return 0. */
int Chain::getKza(int rank)
{

  if (rank >= chainLength)
    return 0;
  else
    return root->getRankKza(rank);
}

int Chain::getRoot() 
{ 
  return root->getKza(); 
}

void Chain::modeReverse()
{
  mode  = MODE_REVERSE;
  NuclearData::modeReverse();
}

bool Chain::checkForDecayLoop()
{
  int idx1,idx2;
  for (idx1 = 0; idx1 < chainLength; idx1++)
    for (idx2 = idx1+1; idx2 < chainLength; idx2++)
      if (l[idx1] == l[idx2])
	return true;

  return false;
}
