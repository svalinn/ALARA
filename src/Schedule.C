/* $Id: Schedule.C,v 1.4 2003-01-13 04:34:58 fateneja Exp $ */
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

#include "Schedule.h"
#include "ScheduleItem.h"
#include "CoolingTime.h"

#include "topSchedule.h"

/***************************
 ********* Service *********
 **************************/

/** When called with no arguments, the default constructor creates a
    blank list head.  Otherwise, it creates and fills the storage for
    'schedName', initializes 'usedAsSub' to FALSE, creates a blank
    list of ScheduleItems, and initializes 'next' to NULL. */
Schedule::Schedule(const char* name)
{
  schedName = NULL;
  if (name != NULL)
    {
      schedName = new char[strlen(name)+1];
      memCheck(schedName,"Schedule::Schedule(...) constructor: schedName");
      strcpy(schedName,name);
    }

  itemListHead = new ScheduleItem(SCHED_HEAD);
  memCheck(itemListHead,"Schedule::Schedule(...) constructor: itemListHead");

  usedAsSub = FALSE;

  next=NULL;
}


/** This constructor is identical to default constructor.  Therefore,
    the name is copied, but the itemList and subsequent list item
    'next' are not.  The 'usedAsSub' variable is specially copied. */
Schedule::Schedule(const Schedule &s)
{
  schedName = NULL;
  if (s.schedName != NULL)
    {
      schedName = new char[strlen(s.schedName)+1];
      memCheck(schedName,"Schedule::Schedule(...) copy constructor: schedName");
      strcpy(schedName,s.schedName);
    }

  itemListHead = new ScheduleItem(SCHED_HEAD);
  memCheck(itemListHead,"Schedule::Schedule(...) copy constructor: itemListHead");

  usedAsSub = FALSE;

  next=NULL;
}


/** Destructor deletes space for 'schedName', deletes itemList
    and then destroys list of Schedules by deleting 'next'. */
Schedule::~Schedule()
{
  delete[] schedName; 
  delete itemListHead; 
  delete next; 
  delete calcSched;
}


/** This assignment operator behaves similarly to the copy
    constructor.  The correct implementation of this operator must
    ensure that previously allocated space is returned to the free
    store before allocating new space into which to copy the
    object. Note that 'next' is NOT copied, the left hand side object
    will continue to be part of the same list unless explicitly
    changed. */
Schedule& Schedule::operator=(const Schedule &s)
{
  if (this == &s)
    return *this;

  schedName = NULL;
  if (s.schedName != NULL)
    {
      schedName = new char[strlen(s.schedName)+1];
      memCheck(schedName,"Schedule::operator=(...): schedName");
      strcpy(schedName,s.schedName);
    }
  
  delete itemListHead;
  itemListHead = new ScheduleItem(SCHED_HEAD);
  memCheck(itemListHead,"Schedule::operator=(...): itemListHead");

  usedAsSub = FALSE;

  return *this;

}

/****************************
 *********** Input **********
 ***************************/

/** A list of schedule items is read until the keyword 'end'. */
Schedule* Schedule::getSchedule(istream& input)
{
  char name[256], token[64];

  input >> name;
  next = new Schedule(name);
  memCheck(next,"Schedule::getSchedule(...): next");


  Schedule *schedPtr = next;

  /* read a list of schedule items until keyword "end" */
  ScheduleItem *itemList = next->itemListHead;

  verbose(2,"Reading items for schedule %s.",name);

  clearComment(input);
  input >> token;
  while (strcmp(token,"end"))
    {
      if (isalpha(token[0]))
	itemList = itemList->getSubSched(token,input);
      else 
	itemList = itemList->getPulse(atof(token),input);

      clearComment(input);
      input >> token;
    }
  
  if (itemList->head())
    warning(200,"Schedule %s is empty",name);


  return schedPtr;         
}

/***************************
 ********* xCheck **********
 **************************/

/** This function sets up the cross-check by looping through the
    schedules, and passing the 2 arguments (the head of the Flux list
    and the head of the History list) on to the list of ScheduleItems. */
void Schedule::xCheck(Flux *fluxHead, History *histHead)
{
  Schedule *head=this;
  Schedule *ptr=this;

  verbose(2,"Checking for all sub-schedules, fluxes and pulsing histories referenced in schedules.");

  while (ptr->next != NULL)
    {
      ptr=ptr->next;
      debug(2,"Checking schedule %s (%x)",ptr->schedName,ptr);
      ptr->itemListHead->xCheck(head,fluxHead,histHead,ptr->schedName);
    }

  verbose(3,"All sub-schedules, fluxes and pulsing histories referenced in schedules were found.");
}

/** This is done to ensure that the parsing of the schedules was done
    correctly. The arguments indicate which level this schedule is at in
    the hierarchy, which history it is pulsed with and the delay after the
    item.  For each ScheduleItem, a similar routine is called, which
    may recursively call back to this routine. */
void Schedule::write(int level, char *histName, double delay, char dUnits)
{
  Schedule *ptr = this;
  int lvlNum;

  /* on first call, search for top level schedule */
  if (level==0)
    {
      verbose(0,"\n\n***Please review this schedule hierarchy.!!!!!!!!!!\n");
      while (ptr->next != NULL)
	{
	  ptr = ptr->next;
	  if (!ptr->usedAsSub)
	    break;
	}
      
      if (ptr->usedAsSub)
	error(400,"Unable to find top level schedule.\nA top level schedule must not used as a sub-schedule.");

      cout << "Schedule '" << ptr->schedName << "':" << endl;
    }
  else
    {
      for (lvlNum=0;lvlNum<level;lvlNum++)
	cout << "\t";
      cout << "Schedule '" << ptr->schedName << "' with " << delay 
	   << " " << dUnits << " delay and pulsed with history '" 
	   << histName << "':" << endl;
    }

  ptr->itemListHead->write(level+1);

  if (level==0)
    verbose(0,"\n***End of schedule hierarchy.\n\n");

}

/****************************
 ********* Preproc **********
 ***************************/

/** Simultaneously, it finds the Schedule object which is at the top of
    the hierarchy and converts it to a topSchedule object (derived from
    calcSchedule) and sets its after-shutdown cooling times from the first
    argument. The pointer to this topSchedule object is returned.  */
topSchedule* Schedule::makeSchedules(CoolingTime *coolList)
{
  Schedule *head = this;
  Schedule *ptr = this;

  topSchedule *top = NULL;

  verbose(2,"Processing schedule hierarchy.");

  /* for each schedule */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;

      verbose(3,"Initializing schedule %s",ptr->schedName);
      /* initialize a new calc schedule object */
      ptr->calcSched = ptr->itemListHead->makeSchedule();

      /* if not used, set this as the top schedule */
      if (!ptr->usedAsSub)
	{
	  delete top;
	  top = new topSchedule(ptr->calcSched,coolList);
	  delete ptr->calcSched;
	  ptr->calcSched = top;
	  verbose(4,"Setting schedule %s as the top schedule.",
		  ptr->schedName);
	}
      
    }

  ptr = head;
  /* for each schedule */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      verbose(3,"Building schedule %s",ptr->schedName);
      /* build the calc schedule from the schedule items */
      ptr->itemListHead->buildSched(ptr->calcSched);
    }

  verbose(3,"Processed all schedules returning top");

  return top;

}



/****************************
 ********* Utility **********
 ***************************/

/** If found, returns the pointer to the appropriate Schedule object,
    otherwise, NULL. */
Schedule* Schedule::find(char *srchSched)
{
  Schedule *ptr=this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (!strcmp(ptr->schedName,srchSched))
	{
	  debug(4,"Found schedule %s: 0x%x (%s)", srchSched,ptr,ptr->schedName);
	  return ptr;
	}
    }

  return NULL;
}
