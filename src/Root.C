/* $Id: Root.C,v 1.15 2003-01-13 04:34:52 fateneja Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "Root.h"
#include "Chain.h"

#include "Mixture.h"

#include "topSchedule.h"

#include "Result.h"

/****************************
 ********* Service **********
 ***************************/
/** Invokes default constructor of base class Node, and sets 'mixList'
    and 'next' to NULL. */
Root::Root() : 
  Node() 
{ 
  nextRoot=NULL; 
  mixList = NULL;
}

/** Invokes copy constructor for base class Node and copies pointer
    'mixList', ie. new mixList points to old mixList, and NOT a copy of the
    mixList. 'next' = NULL. */
Root::Root(const Root& r) : 
  Node(r) 
{ 
  nextRoot=NULL; 
  mixList=r.mixList; 
}

/** Invoks a copy constructor of base class Node with the dereferenced
    first argument and then copying the 'mixList' pointer (see note for
    copy constructor above).  The 'next' pointer is set to the second
    argument. */
Root::Root(Root* cpyRoot, Root* nxtPtr) :
  Node(*cpyRoot)
{
  nextRoot = nxtPtr;
  mixList = new MixCompRef(*(cpyRoot->mixList));
}

Root::Root(char* isoName, double isoDens, Mixture* mix,Component* comp) :
  Node(isoName)
{
  mixList = new MixCompRef(mix,comp,isoDens);
  mix->incrTotalNDensity(isoDens);

  memCheck(mixList,"Root::Root(...) constructor: mixList");

  nextRoot = NULL;

}

/*****************************
 ********* Solution **********
 ****************************/

/* The guts of the solution start here */
/* solve the entire tree for all the roots */
/* called by alara::main(...) */

void Root::solve(topSchedule *schedule)
{
  Root* ptr=this;
  float totalTime, incrTime;
  int firstNode=0,lastNode=0,rootCtr=0;
  int oldChainCtr = 0,totalChainCtr = 0;
  char isoSym[15];

  /* skip over head of rootlist */
  while (ptr != NULL && ptr->kza <1)
    ptr = ptr->nextRoot;


  lastNode = Statistics::numNodes();
  Statistics::cputime(incrTime,totalTime);
  
  /* for each root */
  while (ptr != NULL)
    {
      verbose(2,"Solving Root #%d: %s", ++rootCtr,isoName(ptr->kza,isoSym));

      /* start a new chain */
      Chain *chain = new Chain(ptr,schedule);
      memCheck(chain,"Root::solve(...): chain");

      /* for each chain */
      while (chain->build(schedule)) 
	{
	  totalChainCtr = Statistics::accountChain(chain->getChainLength());
	  chainCode++;
	  chain->setupColRates();
	  /* set the decay matrices for the entire schedule */
	  schedule->setDecay(chain);
	  /* solve the transfer matrices for each mixture with this root */
	  ptr->mixList->solve(chain,schedule);
	}
      delete chain;

      firstNode = lastNode;
      lastNode = Statistics::numNodes();
      Statistics::cputime(incrTime,totalTime);
      verbose(2,"      last Root: %d nodes in %d chains with maximum length %d.",
	      lastNode-firstNode, totalChainCtr-oldChainCtr, 
	      Statistics::accountMaxRank());
      verbose(3,"                 in %0.3f s (%0.3f nodes/s)",incrTime,
	      (lastNode-firstNode)/incrTime);
      verbose(2,"   Total so far: %d nodes in %d chains with maximum length %d.",
	      lastNode,totalChainCtr,Statistics::maxRank());
      verbose(3,"                 in %0.2f s (%0.3f nodes/s)",
	      totalTime,lastNode/totalTime);
      oldChainCtr = totalChainCtr;

      ptr->mixList->writeDump();

      ptr = ptr->nextRoot;
    }
}


/*****************************
 ********* PostProc **********
 ****************************/

/** It returns a pointer to that target isotope and the 'kza' value for
    that target in the first argument. */
Root* Root::readSingleDump(int& getKza)
{
  Root *ptr=this;

  /* skip over head at first call */
  while (ptr != NULL && ptr->kza < 1)
    ptr = ptr->nextRoot;

  if (ptr != NULL)
    {
      verbose(3,"Reading dump for Target: %d",ptr->kza);
      ptr->mixList->readDump(ptr->kza);
      getKza = ptr->kza;
    }
  
  return ptr->nextRoot;
}
      

void Root::readDump()
{
  Root *ptr=this;

  /* skip over head of rootlist */
  while (ptr != NULL && ptr->kza <1)
    ptr = ptr->nextRoot;

  /* for each root */
  while (ptr != NULL)
    {
      verbose(3,"Reading dump for Root: %d",ptr->kza);

      ptr->mixList->readDump(ptr->kza);

      ptr = ptr->nextRoot;
    }

  verbose(2,"Read dump file.");
}

Component* Root::getComp(double &density, Mixture *mix, Component *lastComp)
{
  return mixList->getComp(density,mix,lastComp);
}

/****************************
 ********* Utility **********
 ***************************/

/** It returns the pointer to the matched object, or NULL if no match. */
Root* Root::find(int srchKza)
{
  Root* ptr = this;

  while (ptr != NULL)
    {
      //debug(7,"Comparing to %d (%x)",ptr->kza,ptr);
      if (ptr->kza == srchKza)
	break;
      else
	ptr = ptr->nextRoot;
    }

  debug(7,"Returning %x",ptr);
  return ptr;

}


double Root::maxConc()
{
  return mixList->maxConc();
}

double Root::mixConc(Mixture *mixPtr)
{
  return mixList->mixConc(mixPtr);
}


/*****************************
 *********** List ************
 ****************************/

/** It has already been established that this isotope does not occur
    in the list, therefore, it always results in a new object.  The
    new object may be inserted at the front of the list, in the middle
    of the list or at the end of the list. If the root is added to the head,
    a new head is returned. */
void Root::add(Root* addRoot)
{
  Root* prevRoot = NULL;
  Root* ptr = this;

  //debug(7,"Comparing to %d (%x) with next %x",ptr->kza,ptr,ptr->nextRoot);
  while (ptr->kza < addRoot->kza && ptr->nextRoot != NULL)
    {
      prevRoot = ptr;
      ptr = ptr->nextRoot;
      //debug(7,"Comparing to %d (%x) with next %x",ptr->kza,ptr,ptr->nextRoot);
    }
  
  debug(7,"Final comparison to %d (%x)",ptr->kza,ptr);
  if (ptr->kza > addRoot->kza)
    {
      debug(8,"Inserting new entry between %d and %d",
	    prevRoot->kza, ptr->kza);
      prevRoot->nextRoot = new Root(addRoot,ptr);
    }
  else
    {
      debug(8,"adding new root after %d (%x).",ptr->kza,ptr);
      ptr->nextRoot = new Root(addRoot,NULL);
    }

}

/** For each root in the new list, it if does not exist in the list,
    it is added with add(...).  If it does already exist, its
    mixture/component reference are added with
    MixCompRef::tally(...). */
Root* Root::merge(Root* rootList)
{
  Root* head = this;
  Root *found;


  verbose(4,"Merging Root lists.");

  /* advance past head of root list */
  while (rootList->kza == 0)
    rootList = rootList->nextRoot;
    

  /* for all elements of new root list */
  while (rootList != NULL)
    {
      verbose(5,"Merging root: %d.",rootList->kza);
      /* search for them in the old root list */
      found = head->find(rootList->kza);

      /* if not found, add to list */
      if (found == NULL)
	{
	  debug(6,"Root not found, adding new item.");
	  head->add(rootList);
	  debug(6,"Root not found, added new item.");
	}
      /* if found, tally the mixture and component number */
      else
	{
	  debug(6,"Root found, tallying mix and comp pointers.");
	  found->mixList->tally(rootList->mixList);
	}

      rootList = rootList->nextRoot;
    }

  debug(6,"Finished merging this rootList.");
	
  return head;
  
}
