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

/****************************
 ********* Service **********
 ***************************/

Node::Node(char *isoName)
{
  debug(4,"Making new Node: %s.",isoName);

  kza = 0;
  if (isoName != NULL)
    {
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

Node::Node(int nextKza, Node* passedPrev, double* passedSingle, 
	   int passedRank, int passedState) 
  : NuclearData(passedSingle), TreeInfo(passedPrev,passedRank,passedState), 
    kza(nextKza)
  { }



/****************************
 ********** Chain ***********
 ***************************/

/* function to read nuclear data from arbitrary data source */
/* called by Chain::build() */
void Node::readData()
{
  dataLib->readData(kza,this);

  /* this is only true for forward mode */
  if (nPaths > 0)
    {
      D = paths[nPaths];
      
      /* if a new node is created with TRUNCATE_STABLE state
       * strip its pure transmutation reactions immediately */
      if (state == TRUNCATE_STABLE)
	state = stripNonDecay();
    }
}


double Node::getLambda(int setKza)
{
  kza = setKza;
  readData();

  if (nPaths>0 && D[nGroups]>0)
    return D[nGroups];
  else
    return 0;
}

double Node::getHeat(int setKza)
{
  kza = setKza;
  readData();

  if (nPaths>0 && D[nGroups]>0)
    return D[nGroups] * (E[0]+E[1]+E[2]);
  else
    return 0;
}

double Node::getAlpha(int setKza)
{
  kza = setKza;
  readData();

  if (nPaths>0 && D[nGroups]>0)
    return D[nGroups] * E[0];
  else
    return 0;
}

double Node::getBeta(int setKza)
{
  kza = setKza;
  readData();

  if (nPaths>0 && D[nGroups]>0)
    return D[nGroups] * E[1];
  else
    return 0;
}

double Node::getGamma(int setKza)
{
  kza = setKza;
  readData();

  if (nPaths>0 && D[nGroups]>0)
    return D[nGroups] * E[2];
  else
    return 0;
}

/* function to copy pointers to the chain data */
/* called by Chain::build() */
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
  if (P == NULL)
    {
      rates[rank] = NULL;
      rates[rank+2*step] = NULL;
    }
  else
    {
      rates[rank] = P;
      rates[rank+2*step] = rates[rank]+nGroups;
    }

  /* find natural loop with current isotope */
  loopRank[rank] = findLoop();

  /* make sure that there is no loop within this natural loop
   * i.e. loopRank must monotonically increase */
  if (rank > 0 && loopRank[rank]<loopRank[rank-1] && loopRank[rank-1]>-1)
    loopRank[rank] = loopRank[rank-1];
}

/* re-initialize elements when the chain retracts */
void Node::delRates(double **rates, const int step, int *loopRank)
{
  rates[rank] = NULL;
  rates[rank+step] = NULL;
  rates[rank+2*step] = NULL;
  rates[rank+3*step] = NULL;
  loopRank[rank] = rank;
}


/* function to determine state of tree building process.

 * This function is called twice from Chain::build(...).  The first
 * time is only for new isotopes which, by logic, must be in state
 * CONTINUE or TRUNCATE_STABLE.  This is when the argument is used.
 * The subsequent call is used to determine the action of the
 * Chain::build(...) function.  The arguments defaults to 0 and the
 * state upon entering the function could be one of many.
 */

/* called by Chain::build() */
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
	  else if (stateBits >= TRUNCATE_STABLE)
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
    error(1000,"Programming error: trying to add too many relations in chain.");

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

/* delete everything below the current node */
void Node::prune()
{ 
  delete next; 
  next = NULL; 
};

/* Retract the chain by returning the prev
 * - usually followed by a call to prune() */
Node* Node::retract()
{
  return prev;
}

/* function to search up a chain for a loop */
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
  
/* search down a chain for a certain rank */
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




