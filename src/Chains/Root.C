/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "Root.h"
#include "Chain.h"

#include "Calc/topSchedule.h"
#include "Output/Result.h"

/****************************
 ********* Service **********
 ***************************/
Root::Root() : 
  Node() 
{ 
  nextRoot=NULL; 
  mixList = NULL;
}

Root::Root(const Root& r) : 
  Node(r) 
{ 
  nextRoot=NULL; 
  mixList=r.mixList; 
};

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
      verbose(2,"   last Root: %d nodes in %0.3f s (%0.3f nodes/s)",
	      lastNode-firstNode, incrTime,(lastNode-firstNode)/incrTime);
      verbose(2,"   Total Nodes: %d in %0.2f s (%0.3f nodes/s)",
	      lastNode,totalTime,lastNode/totalTime);

      ptr->mixList->writeDump();

      ptr = ptr->nextRoot;
    }

  Result::resetBinDump();

}


/*****************************
 ********* PostProc **********
 ****************************/

void Root::readDump()
{
  Root *ptr=this;

  /* skip over head of rootlist */
  while (ptr != NULL && ptr->kza <1)
    ptr = ptr->nextRoot;

  /* for each root */
  while (ptr != NULL)
    {
      verbose(2,"Reading dump for Root: %d",ptr->kza);

      ptr->mixList->readDump(ptr->kza);

      ptr = ptr->nextRoot;
    }

}

Component* Root::getComp(double &density, Mixture *mix, Component *lastComp)
{
  return mixList->getComp(density,mix,lastComp);
}

/****************************
 ********* Utility **********
 ***************************/

/* search for a particular kza in a root list */
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


/*****************************
 *********** List ************
 ****************************/

/* add a root to the list:
 *  - the root may be added at the head, in which case
 *    a new head is returned */
void Root::add(Root* addRoot)
{
  Root* prev = NULL;
  Root* ptr = this;

  //debug(7,"Comparing to %d (%x) with next %x",ptr->kza,ptr,ptr->nextRoot);
  while (ptr->kza < addRoot->kza && ptr->nextRoot != NULL)
    {
      prev = ptr;
      ptr = ptr->nextRoot;
      //debug(7,"Comparing to %d (%x) with next %x",ptr->kza,ptr,ptr->nextRoot);
    }
  
  debug(7,"Final comparison to %d (%x)",ptr->kza,ptr);
  if (ptr->kza > addRoot->kza)
    {
      debug(8,"Inserting new entry between %d and %d",
	    prev->kza, ptr->kza);
      prev->nextRoot = new Root(addRoot,ptr);
    }
  else
    {
      debug(8,"adding new root after %d (%x).",ptr->kza,ptr);
      ptr->nextRoot = new Root(addRoot,NULL);
    }

}

/* merge two lists */
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
