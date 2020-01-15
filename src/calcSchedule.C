/* $Id: calcSchedule.C,v 1.8 2003-01-13 04:34:29 fateneja Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Preproc: functions directly related to preprocessing of input
 *          prior to solution 
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "calcSchedule.h"
#include "calcScheduleT.h"
#include "PulseHistory.h"

#include "Input/History.h"


/***************************
 ********* Service *********
 **************************/

/** When called with no arguments, this constructor makes an empty
    object with various members set to 0 or NULL except:
    calcSchedule::D is set to an Identity matrix of size 0, and
    calcSchedule::fluxCode is initialized to -1.  Otherwise,
    calcSchedule::nItems is initialized with the argument and
    storage is allocated for the calcSchedule::nItems pointers in
    calcSchedule::subSched. (TYPE A in description above.) */
calcSchedule::calcSchedule(int numItems)
{
  setCode = 0;
  history = NULL;
  delay = 0;
  fluxCode = -1;
  opTime = 0;
  subSched = NULL;

  nItems = numItems;
  if (nItems>0)
    {
      subSched = new calcSchedule*[nItems];
      memCheck(subSched,"calcSchedule::calcSchedule(...) constructor: subSched");

      for (int itemNum=0;itemNum<nItems;itemNum++)
	subSched[itemNum] = NULL;
    }

}

/** Copies everything on a member-by-member basis except 'subSched'
    which is copied on an element-by-element basis.  Note that each
    pointer of 'subSched' is copied, and not the objects which each
    of the pointers points to. */
calcSchedule::calcSchedule(const calcSchedule& c)
{
  setCode = c.setCode;
  nItems = c.nItems;
  
  delay = c.delay;
  D = c.D;
  history = c.history;

  opTime = c.opTime;
  fluxCode = c.fluxCode;
  
  if (nItems>0)
    {  
      subSched = new calcSchedule*[nItems];
      memCheck(subSched,
	       "calcSchedule::calcSchedule(...) copy constructor: subSched");
    
      for (int itemNum=0;itemNum<nItems;itemNum++)
	subSched[itemNum] = c.subSched[itemNum];
    }
}  

/** The primary characteristics of a single pulse calcSchedule object
    are that the 'nItems' is set to 0, and 'fluxCode' and 'opTime'
    are set.  (TYPE B-0 in description above.) */
calcSchedule::calcSchedule(double td, double pulseTime, History* hist,
			   int fluxNum)
{
  setCode = 0;
  nItems = 0;

  delay = td;
  history = hist->getCalcHist();

  opTime = pulseTime;
  fluxCode = fluxNum;

  subSched = NULL;

}

/** The primary characteristics of this kind of calcSchedule object
    are that 'nItems' is set to 1, 'subSched' has dimension 1, and
    its first/only element points to the subsequent calcSchedule
    object. (TYPE B-1 in class description.) */
calcSchedule::calcSchedule(double td, History* hist, calcSchedule* schedItem)
{
  setCode = 0;
  nItems = 1;

  delay = td;
  history = hist->getCalcHist();

  opTime = -1;
  fluxCode = -1;

  subSched = new calcSchedule*;
  memCheck(subSched,"calcSchedule::calcSchedule(...) constructor: subSched");

  subSched[0] = schedItem;

}

/** Deletes the storage for 'subSched' pointers but not the objects
    that they point to!  This does not destroy the entire 
    hierarchy. */
calcSchedule::~calcSchedule()
{
  int itemNum;

  if (nItems>0)
    for (itemNum=0;itemNum<nItems;itemNum++)
      delete subSched[itemNum];

  delete[] subSched;
}

/** The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.  Note that
    each pointer of 'subSched' is copied, and not the objects which
    each of the pointers points to. */
calcSchedule& calcSchedule::operator=(const calcSchedule& c)
{
  if (this == &c)
    return *this;

  setCode = c.setCode;

  delay = c.delay;
  D = c.D;
  history = c.history;

  opTime = c.opTime;
  fluxCode = c.fluxCode;

  while (nItems-->0)
    delete subSched[nItems];
  delete subSched;
  subSched = NULL;

  nItems = c.nItems;
  
  if (nItems>0)
    {  
      subSched = new calcSchedule*[nItems];
      memCheck(subSched,"calcSchedule::operator=(...): subSched");
      
      for (int itemNum=0;itemNum<nItems;itemNum++)
	subSched[itemNum] = c.subSched[itemNum];
    }

  return *this;
}  

/***************************
 ********* Preproc *********
 **************************/

/* All schedules with a single item can be collapsed.
 * The new schedule has the same properties as the sub-item
 * EXCEPT, the delay of the original schedule and a history
 * made by merging the original schedule's history with the 
 * sub-item's history.
   See point (4.) above for more details on the collapsing
   algorithm. */
void calcSchedule::collapse()
{
  int itemNum;

  verbose(3,"Examining schedule with %d items",nItems);

  while (nItems == 1)
    {
      verbose(4,"Collapsing this schedule with sub-schedule with %d items.",
	      subSched[0]->nItems);
      nItems = subSched[0]->nItems;
      history = new PulseHistory(subSched[0]->history,subSched[0]->delay,
				 history);
      memCheck(history,"calcSchedule::collapse(...): history");
      opTime = subSched[0]->opTime;
      fluxCode = subSched[0]->fluxCode;
      calcSchedule **tmp = subSched;
      subSched = subSched[0]->subSched;
      delete[] tmp;

    }

  for (itemNum=0;itemNum<nItems;itemNum++)
    subSched[itemNum]->collapse();

}


/***************************
 ******** Solution *********
 **************************/
/** After confirming that this schedule has not already been
    processed, the decay matrix 'D' for this schedule is set,
    PulseHistory::setDecay(...) is called on this calcSchedule's
    'history' member, and the recursion takes place. */
void calcSchedule::setDecay(Chain* chain)
{
  int itemNum;

  if (setCode != chainCode)
    {
      chain->setDecay(D,delay);
      history->setDecay(chain);

      if (nItems>0)
	for (itemNum=0;itemNum<nItems;itemNum++)
	  subSched[itemNum]->setDecay(chain);

      setCode = chainCode;
    }
  
}

/** This small code segment is separated out of setT(...) to allow
    it to be called through a topSchedule derived class object
    and still access the appropriate members. */
void calcSchedule::setSubTs(Chain* chain, calcScheduleT *schedT)
{
  int itemNum=0;

  subSched[itemNum]->setT(chain,(*schedT)[itemNum]);
  schedT->opBlock() = (*schedT)[itemNum]->total();
  for (itemNum=1;itemNum<nItems;itemNum++)
    {
      subSched[itemNum]->setT(chain,(*schedT)[itemNum]);
      schedT->opBlock() = (*schedT)[itemNum]->total() * schedT->opBlock();
    }
}

/** If compound schedule, setSubTs(...) is called to recurse and build
    master transfer matrix. If single pulse (end state of recursion),
    'Chain::fillTMat(...)' is called to fill the 'opBlockT' transfer
    matrix.  Following this, the pulsing history is applied to fill
    'calcScheduleT::histT' and then the final delay on the schedule is
    applied to fill 'calcScheduleT::totalT'. */
void calcSchedule::setT(Chain* chain, calcScheduleT *schedT)
{
  if (nItems>0)
    setSubTs(chain,schedT);
  else
    chain->fillTMat(schedT->opBlock(),opTime,fluxCode);

  schedT->hist() = history->doHistory(schedT->opBlock());

  chain->mult(schedT->total(),D,schedT->hist());


}
