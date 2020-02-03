/* $Id: PulseLevel.C,v 1.5 2003-01-13 04:34:58 fateneja Exp $ */
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

#include "PulseHistory.h"

/***************************
 ********* Service *********
 **************************/

/** This assignmnent operator behaves similarly to the copy
    constructor.  The correct implementation of this operator must
    ensure that previously allocated space is returned to the free
    store before allocating new space into which to copy the
    object. Note that 'next' is NOT copied, the object will continue to
    be part of the same list unless explicitly changed. */
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

/** It returning a pointer to the newly created PulseLevel object. The
    first argument is the number of pulses, which must be read by the
    calling function before calling this function. */
PulseLevel* PulseLevel::getPulseLevel(int inNumPulse,istream& input)
{
  double inDelay;
  char inUnits;
  
  input >> inDelay >> inUnits;
  
  /* check for valid units */
  if (strchr(UNITS,inUnits) == NULL)
    error(190,"Invalid units in pulse level: %10g %c",
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

/** This function should be called through the head of the PulseLevel
    list within a particular History object. A pointer to the new object
    is returned. */
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


