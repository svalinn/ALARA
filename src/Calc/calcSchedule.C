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

/* create a single pulse item */
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

/* create a sub-schedule item */
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

calcSchedule::~calcSchedule()
{
  int itemNum;

  if (nItems>0)
    for (itemNum=0;itemNum<nItems;itemNum++)
      delete subSched[itemNum];

  delete subSched;
}

calcSchedule& calcSchedule::operator=(const calcSchedule& c)
{
  if (this == &c)
    return *this;

  setCode = c.setCode;
  nItems = c.nItems;
  
  delay = c.delay;
  D = c.D;
  history = c.history;

  opTime = c.opTime;
  fluxCode = c.fluxCode;
  
  delete [] subSched;
  subSched = NULL;

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
 * sub-item's history. */
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
      delete tmp;

    }

  for (itemNum=0;itemNum<nItems;itemNum++)
    subSched[itemNum]->collapse();

}


/***************************
 ******** Solution *********
 **************************/

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


void calcSchedule::setSubTs(Chain* chain, calcScheduleT *schedT)
{
  int itemNum;

  for (itemNum=0;itemNum<nItems;itemNum++)
    {
      subSched[itemNum]->setT(chain,(*schedT)[itemNum]);
      schedT->opBlock() = (*schedT)[itemNum]->total() * schedT->opBlock();
    }
}

void calcSchedule::setT(Chain* chain, calcScheduleT *schedT)
{
  if (nItems>0)
    setSubTs(chain,schedT);
  else
    chain->fillTMat(schedT->opBlock(),opTime,fluxCode);

  schedT->hist() = history->doHistory(schedT->opBlock());

  chain->mult(schedT->total(),D,schedT->hist());


}
