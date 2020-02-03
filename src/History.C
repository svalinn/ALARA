/* $Id: History.C,v 1.4 2003-01-13 04:34:56 fateneja Exp $ */
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

#include "History.h"
#include "PulseLevel.h"
#include "Schedule.h"

#include "PulseHistory.h"

/***************************
 ********* Service *********
 **************************/
/** This constructor creates a blank list head with no arguments.
    Otherwise, it sets the name of the history and initializes the
    pulse level list with a head item.  Other pointers are set to
    NULL. */
History::History(const char* name)
{
  histName = NULL;
  if (name != NULL)
    {
      histName = new char[strlen(name)+1];
      memCheck(histName,"History::History(...) constructor: histName");
      strcpy(histName,name);
    }

  pulseLevelHead = new PulseLevel(PULSE_HEAD);
  memCheck(pulseLevelHead,"History::History(...) constructor: pulseLevelHead");

  calcHist = NULL;

  next = NULL;
}

/** This constructor is identical to the default constructor.  While
    the name is copied, the pulse level list is initialized with a new
    list head, but neither the pulsing information nor the 'calcHist'
    pointer are copied! */
History::History(const History& h)
{
  histName = NULL;
  if (h.histName != NULL)
    {
      histName = new char[strlen(h.histName)+1];
      memCheck(histName,"History::History(...) copy constructor: histName");
      strcpy(histName,h.histName);
    }

  pulseLevelHead = new PulseLevel(PULSE_HEAD);
  memCheck(pulseLevelHead,"History::History(...) copy constructor: pulseLevelHead");

  calcHist = NULL;

  next = NULL;
}

/** Destructor deletes the storage for 'histName' and deletes the
    pulsing information list.  It then destroys the rest of the list
    by deleting 'next'. */
History::~History()
{ 
  delete histName; 
  delete pulseLevelHead; 
  delete next;
}  

/** This assignment operator behaves the similarly to the copy
    constructor.  Even though new puleList and calcHist information is
    not copied from the righ hand side, the old values are deleted.
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the left hand side object will continue to
    be part of the same list unless explicitly changed. */
History& History::operator=(const History& h)
{
  if (this == &h)
    return *this;

  delete histName;
  histName = NULL;
  if (h.histName != NULL)
    {
      histName = new char[strlen(h.histName)+1];
      memCheck(histName,"History::operator=(...): histName");
      strcpy(histName,h.histName);
    }

  delete pulseLevelHead;
  pulseLevelHead = new PulseLevel(PULSE_HEAD);
  memCheck(pulseLevelHead,"History::operator=(...): pulseLevelHead");

  delete calcHist;
  *calcHist = *(h.calcHist);

  return *this;

}

/****************************
 *********** Input **********
 ***************************/

/* get a single pulsing history */
/* called by Input::read(...) */
/** After reading the descriptive name of the history, it read the
    pulsing level information into the pulsing level list.  Returns a
    pointer to the newly created History object. */
History* History::getHistory(istream& input)
{
  char name[256], token[64];

  input >> name;
  next = new History(name);
  memCheck(next,"History::getHistory(...): next");

  History* histPtr = next;

  /* read a list of pulse level information until keyword "end" */
  PulseLevel* pulseLevel = histPtr->pulseLevelHead;

  verbose(2,"Reading pulsing levels for History %s:",name);

  clearComment(input);
  input >> token;
  while (strcmp(token,"end"))
    {
      pulseLevel = pulseLevel->getPulseLevel(atoi(token),input);

      clearComment(input);
      input >> token;
    }
  
  if (pulseLevel->head())
    warning(160,"History %s is empty",name);

  return histPtr;         
}

/****************************
 ********* Preproc **********
 ***************************/

/* convert the histories as input into useful objects */
/* called by Input::preproc(...) */
/** This function is called through the object at the head of the history
    list. It acts on the entire history list, one at a time, and should
    be called through the head of the history list. It calls
    PulseLevel::makeHistory() through the head of the pulse level
    list. */
void History::makeHistories()
{

  History *ptr = this;

  verbose(2,"Processing pulsing histories.");

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      verbose(3,"Building pulsing history %s",ptr->histName);
      ptr->calcHist = ptr->pulseLevelHead->makeHistory();
    }

  verbose(3,"Processed all the pulsing histories.");
}



/****************************
 ********* Utility **********
 ***************************/

/** If found, a pointer to that history is returned, and if not,
    NULL. */
 History* History::find(char *srchHist)
{
  History *ptr=this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (!strcmp(ptr->histName,srchHist))
	return ptr;
    }

  return NULL;
}
