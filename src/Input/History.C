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

#include "Calc/PulseHistory.h"

/***************************
 ********* Service *********
 **************************/
History::History(char* name)
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

History::~History()
{ 
  delete histName; 
  delete pulseLevelHead; 
  delete next;
}  

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

/* find a requested pulse history definition */
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



