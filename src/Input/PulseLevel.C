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


#include "PulseLevel.h"
#include "Input_def.h"

#include "Calc/PulseHistory.h"

/***************************
 ********* Service *********
 **************************/

PulseLevel::PulseLevel(int inNumPulse, double inDelay, char inUnits) :
  nPulses(inNumPulse),  units(inUnits),  delay(inDelay)
{
  next = NULL;
}

PulseLevel::PulseLevel(const PulseLevel& p) :
  nPulses(p.nPulses),  units(p.units),  delay(p.delay)
{
  next = NULL;
}

PulseLevel& PulseLevel::operator=(const PulseLevel& p)
{
  if (this == &p)
    return *this;

  nPulses = p.nPulses;
  delay = p.delay;
  units = p.units;

  return *this;


}

/****************************
 *********** Input **********
 ***************************/

/* get single pulsing level */
/* called by History::getHistory(...) */
PulseLevel* PulseLevel::getPulseLevel(int inNumPulse,istream& input)
{
  double inDelay;
  char inUnits;

  input >> inDelay >> inUnits;

  /* check for valid units */
  if (strchr(UNITS,inUnits) == NULL)
    error(147,"Invalid units in pulse level: %10g %c",
	  inDelay, inUnits);
  
  next = new PulseLevel(inNumPulse,inDelay,inUnits);
  memCheck(next,"PulseLevel::getPulseLevel(...): next");

  verbose(3,"Adding level with %d pulses and %g %c between pulses.",
	  inNumPulse,inDelay,inUnits);

  return next;

}

/****************************
 ********* Preproc **********
 ***************************/


/* convert the individual pulsing levels into useful objects */
/* called by History::makeHistories(...) */
PulseHistory* PulseLevel::makeHistory()
{
  PulseLevel* head = this;
  PulseLevel* ptr = head;
  int *nPulse, nLevels = 0;
  double *td;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      nLevels++;
    }

  nPulse = new int[nLevels];
  memCheck(nPulse,"PulseLevel::makeHistoryl(...): nPulse");

  td = new double[nLevels];
  memCheck(td,"PulseLevel::makeHistoryl(...): td");

  ptr = head;
  nLevels = 0;
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      nPulse[nLevels] = ptr->nPulses;
      td[nLevels++] = convertTime(ptr->delay,ptr->units);
    }

  verbose(4,"Creating new PulseHistory with %d levels.",nLevels);

  PulseHistory* phPtr = new PulseHistory(nLevels,nPulse,td);
  memCheck(phPtr,"PulseLevel::makeHistoryl(...): phPtr");
  

  return phPtr;
}


