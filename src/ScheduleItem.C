/* $Id: ScheduleItem.C,v 1.6 2003-01-13 04:34:59 fateneja Exp $ */
/* (Potential) File sections:
 * Service: constructors, destructors
 * Input: functions directly related to input of data 
 * xCheck: functions directly related to cross-checking the input
 *         against itself for consistency and completeness
 * Preproc: functions directly related to preprocessing of input
 *          prior to solution 
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 */


#include "ScheduleItem.h"
#include "History.h"
#include "Schedule.h"
#include "Flux.h"
#include "calcSchedule.h"

/***************************
 ********* Service *********
 **************************/

/** When called with no arguments, this default constructor creates a
    blank list head with no data.  Otherwise, is creates and fills
    storage for 'itemName' (acting as 'fluxName' where necessary) and
    'pulseName', and initializes the other variables (perhaps with
    default values), with 'next' set to NULL. */
ScheduleItem::ScheduleItem(int inType, char *name, char *pname, 
			   double inDelay, char inDUnits,
			   double inOpTime, char inOpUnits) :
  type(inType), delay(inDelay), opTime(inOpTime), dUnits(inDUnits),
  opUnits(inOpUnits)
{
  itemName = NULL;
  pulseName = NULL;

  if (name != NULL)
    {
      itemName = new char[strlen(name)+1];
      memCheck(itemName,"ScheduleItem::ScheduleItem(...) constructor: itemName");
      strcpy(itemName,name);
    }

  if (pname != NULL)
    {
      pulseName = new char[strlen(pname)+1];
      memCheck(pulseName,"ScheduleItem::ScheduleItem(...) constructor: pulseName");
      strcpy(pulseName,pname);
    }

  next = NULL;
}

/****************************
 *********** Input **********
 ***************************/

/** The 'itemName' has already been read by the calling routine, and is
    passed as the first argument.  A pointer to the newly created
    ScheduleItem object is returned. */
ScheduleItem* ScheduleItem::getSubSched(char *name,istream& input)
{

  char pname[256];
  double inDelay;
  char inDUnits;

  input >> pname >> inDelay >> inDUnits;
  /* check for valid units */
  if (strchr(UNITS,inDUnits) == NULL)
    error(210,"Invalid units in schedule item delay time: %10g %c",
	  inDelay, inDUnits);
  
  next = new ScheduleItem(SCHED_SCHED,name,pname,inDelay,inDUnits);
  memCheck(next,"ScheduleItem::getSubSched(...): next");

  verbose(3,"Added subschedule %s with pulsing history %s and delay %g %c.",
	  name, pname, inDelay, inDUnits);

  return next;

}

/** The 'opTime' has already been read by the calling routine, and is
    passed as the first argument.  A pointer to the newly created
    ScheduleItem object is returned. */
ScheduleItem* ScheduleItem::getPulse(double inOpTime,istream& input)
{

  char flxName[256], pname[256];
  double inDelay;
  char inDUnits, inOpUnits;

  input >> inOpUnits >> flxName >> pname >> inDelay >> inDUnits;

  /* check for valid units */
  if (strchr(UNITS,inDUnits) == NULL)
    error(210,"Invalid units in schedule item delay time: %10g %c",
	  inDelay, inDUnits);
  if (strchr(UNITS,inOpUnits) == NULL)
    error(211,"Invalid units in single pulse time: %10g %c",
	  inOpTime, inOpUnits);

  next = new ScheduleItem(SCHED_PULSE,flxName,pname,inDelay,inDUnits,
			  inOpTime,inOpUnits);
  memCheck(next,"ScheduleItem::getPulse(...): next");

  verbose(3,"Added single pulse of length %g %c, using flux %s, pulsing history %s and delay %g %c.",
	  inOpTime, inOpUnits, flxName, pname, inDelay, inDUnits);

  return next;

}

/***************************
 ********* xCheck **********
 **************************/

/** Once they are found, named references are usually converted to
    object pointers.  For single pulses, the flux is checked first,
    replacing the 'fluxName' with the 'fluxNum' in the union.  For
    sub-schedules, the referenced schedule is checked, replacing the
    'itemName' with the 'subSched' pointer in the union. For both cases,
    the history is checked, replacing the 'pulseName'with the 'hist'
    pointer in the union.  The arguments are the head items of the lists
    of Schedule, Flux and History, respectively, followed by the name of
    the current schedule, for use in error reporting. */
void ScheduleItem::xCheck(Schedule* schedHead, Flux *fluxHead, History *histHead, char *schedName)
{
  ScheduleItem *ptr=this;

  verbose(3,"Checking cross-referencing for schedule %s.",schedName);

  while (ptr->next != NULL)
    {
      ptr=ptr->next;
      switch (ptr->type)
	{
	case SCHED_PULSE: /* simple pulses */
	  {
	    verbose(4,"Checking for flux %s and pulsing history %s in single pulse item.",
		    ptr->fluxName, ptr->pulseName);
	    /* set ordinal flux number */
	    int tmpFlux = fluxHead->find(ptr->fluxName);
	    /* check that flux is valid */
	    switch (tmpFlux)
	      {
	      case FLUX_NOT_FOUND:
		error(410,"Flux %s for simple pulse item of schedule %s does not exist.",ptr->fluxName,schedName);
		break;
	      case FLUX_BAD_FNAME:
		error(411,"Bad flux file for flux %s for simple pulse item of schedule %s.",
			ptr->fluxName,schedName);
		break;
	      default:
		verbose(5,"Flux %s in single pulse item was found.",ptr->fluxName);
		delete[] ptr->fluxName;
		ptr->fluxNum = tmpFlux;
	      }
	    break;
	  }
	case SCHED_SCHED: /* sub-schedule */
	  {
	    if (!strcmp(ptr->itemName,schedName))
	      error(412,"Schedule recursion: %s.",schedName);

	    verbose(4,"Checking for sub-schedule %s and pulsing history %s.",ptr->itemName,ptr->pulseName);
	    /* set pointer to sub-schedule */
	    Schedule* tmpSched = schedHead->find(ptr->itemName);
	    if (tmpSched == NULL)
	      error(413,"Schedule %s for subschedule item of schedule %s does not exist.",ptr->itemName,schedName);
	    else 
	      {
		verbose(5,"Sub-schedule %s was found.",ptr->itemName);
		delete[] ptr->itemName;
		ptr->subSched = tmpSched;
	      }
	    /* make note that this schedule is used as a sub-schedule */
	    ptr->subSched->use();
	    break;
	  }
	}

      /* set pointer to pulse history */
      History *tmpHist = histHead->find(ptr->pulseName);
      if (tmpHist == NULL)
	error(414,"Pulse history %s for item of schedule %s does not exist.",
	      ptr->pulseName, schedName);
      else
	{
	  verbose(5,"Pulsing history %s was found.",ptr->pulseName);
	  delete[] pulseName;
	  ptr->hist = tmpHist;
	}
    }
}

/** It uses the argument to indicate the tabbing for hierarchical
    representation.  Single pulse items have their time, delay and
    pulsing name written, while sub-schedules recursively call back to
    the Schedule::write(int) function. */
void ScheduleItem::write(int level)
{
  ScheduleItem *ptr = this;
  int lvlNum;

  while (ptr->next != NULL)
    {
      ptr= ptr->next;
      switch (ptr->type)
	{
	case SCHED_PULSE: /* single pulse */
	  for (lvlNum=0;lvlNum<level;lvlNum++)
	    cout << "\t";
	  cout << "pulse: " << ptr->opTime << " " << ptr->opUnits 
	       << " with " << ptr->delay << " " << ptr->dUnits 
	       << " delay pulsed with history "  << ptr->hist->getName() << endl;
      cout << "\t";   
	  cout << "pulse_entry: " << ptr->opTime << " " << ptr->opUnits 
         << " pulse_history: "  << ptr->hist->getName()
	       << " delay " << ptr->delay << " " << ptr->dUnits << endl;
	  break;
	case SCHED_SCHED: /* sub-schedule: recursively call back to schedule::write() */
	  ptr->subSched->write(level,ptr->hist->getName(),ptr->delay,ptr->dUnits);
	  break;
	}
    }
}

/****************************
 ********* Preproc **********
 ***************************/

/** In all cases, a new calcSchedule object is created and passed to
    function Schedule::add(int,*calcSchedule) for inclusion in the list
    of sub-schedules.  NOTE: For each sub-schedule type item, this
    results in an extra calcSchedule object with a single
    sub-schedule.  While it is tempting to collapse this extra object
    with its sub-schedule immediately, the calcSchedule object
    corresponding to the sub-schedule may not be set yet.  This is
    done later (see calcSchedule::collapse()). */
void ScheduleItem::buildSched(calcSchedule *sched)
{
  ScheduleItem *ptr = this;
  int itemNum = 0;

  /* for each item */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      debug(4,"Adding sub-item with type %d.",ptr->type);
      if (ptr->type == SCHED_PULSE)
	{
	  verbose(4,"Adding new pulse item with optime %g %c, delay %g %c, flux %d and history %s",ptr->opTime,ptr->opUnits,ptr->delay,ptr->dUnits,ptr->fluxNum,ptr->hist->getName());
	/* add a normal pulse item */
	  sched->add(itemNum++, 
		     new calcSchedule(convertTime(ptr->delay,ptr->dUnits),
				      convertTime(ptr->opTime,ptr->opUnits),
				      ptr->hist,ptr->fluxNum));
	}
      else
	{
	  verbose(4,"Adding new sub-schedule item %s with delay %g %c, and history %s",
		  ptr->subSched->getName(),ptr->delay,ptr->dUnits, 
		  ptr->hist->getName());
	  /* add a sub-schedule item */
	  sched->add(itemNum++,
		     new calcSchedule(convertTime(ptr->delay,ptr->dUnits),
				      ptr->hist,ptr->subSched->getCalcSched()));
	}
    }
}

/** It does this by counting the number of ScheduleItems in a list, and
    creating the calcSchedule with that number of sub-schedules.  A
    pointer to the newly created calcSchedule object is returned. */
calcSchedule* ScheduleItem::makeSchedule()
{
  ScheduleItem *head = this;
  ScheduleItem *ptr = head;
  int nItems = 0;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      nItems++;
    }

  return new calcSchedule(nItems);
}
