/* $Id: topSchedule.C,v 1.4 2003-01-13 04:34:30 fateneja Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "topSchedule.h"
#include "topScheduleT.h"
#include "PulseHistory.h"

#include "Input/CoolingTime.h"

#include "Chains/Chain.h"

#include "Output/Result.h"

/****************************
 ********* Service **********
 ***************************/
/** This is used to convert a calcSchedule object, pointed to by
    the first argument, to a topSchedule object.  At the same time,
    this function is initializes the number of after-shutdown
    cooling times of interest.  The second argument is the head of
    the list of input coolingTimes which is parsed to set the local
    member.  After establishing the number of cooling times, space
    is allocated and initialized for 'coolD'. */
topSchedule::topSchedule(calcSchedule *sched, CoolingTime *coolList) :
  calcSchedule(*sched)
{
  coolingTime = NULL;
  coolD = NULL;

  nCoolingTimes = coolList->makeCoolingTimes(coolingTime);

  if (nCoolingTimes > 0)
    {
      coolD = new Matrix[nCoolingTimes];
      memCheck(coolD,"topSchedule::topSchedule(...) constructor: coolD");
    }

  topScheduleT::setNumCoolingTimes(nCoolingTimes);
  Result::setNResults(nCoolingTimes+1);

  verbose(3,"Set number of cooling times to %d.",nCoolingTimes);

}  

/** This constructor first invokes its base class copy constructor.
    It then copies the number of cooling times, copying 'coolD' and
    'coolingTime' on an element-by-element basis if it is greater
    than 0. */
topSchedule::topSchedule(const topSchedule& t) :
  calcSchedule(t)
{

  coolingTime = NULL;
  coolD = NULL;
  nCoolingTimes = t.nCoolingTimes;

  if (nCoolingTimes > 0)
    {
      coolD = new Matrix[nCoolingTimes];
      memCheck(coolD,"topSchedule::topSchedule(...) copy constructor: coolD");
      coolingTime = new double[nCoolingTimes];
      memCheck(coolingTime,
	       "topSchedule::topSchedule(...) copy constructor: coolingTime");
      
      for (int coolNum=0;coolNum<nCoolingTimes;coolNum++)
	{
	  coolD[coolNum] = t.coolD[coolNum];
	  coolingTime[coolNum] = t.coolingTime[coolNum];
	}
    }
}

topSchedule::~topSchedule()
{
  delete [] coolD;
  delete coolingTime;
}

/** The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. */
topSchedule& topSchedule::operator=(const topSchedule& t)
{

  if (this == &t)
    return *this;

  /**** BEGIN copied directly from calcSchedule.C ***/
  setCode = t.setCode;
  
  delay = t.delay;
  D = t.D;
  history = t.history;

  opTime = t.opTime;
  fluxCode = t.fluxCode;
  
  while (nItems-->0)
    delete subSched[nItems];
  delete subSched;
  subSched=NULL;

  nItems = t.nItems;

  if (nItems>0)
    {  
      subSched = new calcSchedule*[nItems];
      memCheck(subSched,"calcSchedule::operator=(...): subSched");
      
      for (int itemNum=0;itemNum<nItems;itemNum++)
	subSched[itemNum] = t.subSched[itemNum];
    }
  /**** END copied directly from calcSchedule.C ***/

  delete coolingTime;
  delete [] coolD;

  coolingTime = NULL;
  coolD = NULL;
  nCoolingTimes = t.nCoolingTimes;

  if (nCoolingTimes > 0)
    {
      coolD = new Matrix[nCoolingTimes];
      memCheck(coolD,"topSchedule::operator=(...): coolD");
      coolingTime = new double[nCoolingTimes];
      memCheck(coolingTime,
	       "topSchedule::operator=(...): coolingTime");
      
      for (int coolNum=0;coolNum<nCoolingTimes;coolNum++)
	{
	  coolD[coolNum] = t.coolD[coolNum];
	  coolingTime[coolNum] = t.coolingTime[coolNum];
	}
    }

  return *this;

}


/****************************
 ******** Solution **********
 ***************************/
/** It does not set a decay matrix for the delay since no delay is
    applied to a topSchedule.  It checks for the existence of a
    history before calling setDecay on it, since a topSchedule may
    not have a pulsing history.  It calls Chain::setDecay for each of
    the 'coolD' matrices. */
void topSchedule::setDecay(Chain* chain)
{
  int itemNum, coolNum;

  /* NOTE: topSchedule's don't apply any final decay block */

  if (nItems>0)
    for (itemNum=0;itemNum<nItems;itemNum++)
      subSched[itemNum]->setDecay(chain);
  
  if (history != NULL)
    history->setDecay(chain);

  for (coolNum=0;coolNum<nCoolingTimes;coolNum++)
    chain->setDecay(coolD[coolNum],coolingTime[coolNum]);
  
}

/** It does not apply any final decay operation.  It checks for the
    existence of the pulsing history before applying it.  It applies
    decays for each of the after-shutdown cooling times, storing them
    in the special matrices of the topScheduleT object passed in the
    second argument. */
void topSchedule::setT(Chain* chain, topScheduleT *schedT)
{
  int coolNum;

  if (nItems>0)
    setSubTs(chain, schedT);
  else
    chain->fillTMat(schedT->opBlock(),opTime,fluxCode);

  /* NOTE: Only a topSchedule can be without a pulsing history */
  if (history != NULL)
    schedT->total() = history->doHistory(schedT->opBlock());
  else
    schedT->total() = schedT->opBlock();

  /* ALSO NOTE: topSchedule's don't apply any final decay block */

  for (coolNum=0;coolNum<nCoolingTimes;coolNum++)
    chain->mult(schedT->cool(coolNum),coolD[coolNum],schedT->total());

}

