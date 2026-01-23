/* $Id: PulseHistory.C,v 1.3 2003-01-13 04:34:27 fateneja Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "PulseHistory.h"
#include <iostream>

#include "Chain.h"

/****************************
 ********* Service **********
 ***************************/

/** When called with no arguments, the default constructor sets the
    'nLevels' to and 'nPulse' and 'td' to NULL.  Otherwise, they
    are set with the arguments.  Note that the pointers are copied,
    and not the arrays themselves.  In both cases, 'setCode' is
    initialized to -1. */
PulseHistory::PulseHistory(const char* name, int nlvls, int *pulse, double *decay) :
  setCode(-1), nLevels(nlvls),  nPulse(pulse),  td(decay)
{
  this->histName = NULL;
  if (name != NULL)
    {
      this->histName = new char[strlen(name) + 1];
      memCheck(histName, "PulseHistory::PulseHistory: histName");
      strcpy(this->histName, name);
    }

  D = NULL;
  if (nLevels>0)
    {
      D = new Matrix[nLevels];
      memCheck(D,"PulseHistory::PulseHistory(...) constructor: D");
    }


}

/** Copies all members including an element-by-element copies of
    the two arrays. */
PulseHistory::PulseHistory(const PulseHistory &p) : 
  setCode(p.setCode),nLevels(p.nLevels)
{
  int lvlNum;

  histName = NULL;
  if (p.histName != NULL)
    {
      histName = new char[strlen(p.histName) + 1];
      memCheck(histName, "PulseHistory copy ctor: histName");
      strcpy(histName, p.histName);
    }

  nPulse = NULL;
  td = NULL;
  D = NULL;

  if (nLevels>0)
    {
      nPulse = new int[nLevels];
      memCheck(nPulse,"PulseHistory::PulseHistory(...) copy constructor: nPulse");
      td = new double[nLevels];
      memCheck(td,"PulseHistory::PulseHistory(...) copy constructor: td");
      D = new Matrix[nLevels];
      memCheck(D,"PulseHistory::PulseHistory(...) copy constructor: D");
      
      for (lvlNum=0;lvlNum<p.nLevels;lvlNum++)
	{
	  nPulse[lvlNum] = p.nPulse[lvlNum];
	  td[lvlNum] = p.td[lvlNum];
	  D[lvlNum] = p.D[lvlNum];
	}

    }


}

/** This action is valid (and invoked) whenever a schedule has a
    single sub-schedule.  Basically, a pulsing history with N levels,
    followed by a delay of time D, all pulsed with a history of M
    levels is the same as a pulsing history with N+M+1 levels where
    level N+1 has one pulse and delay time D.  It is possible for N,M
    and/or D to be zero (N+M+D is always > 0), and these cases are
    taken care of. */
PulseHistory::PulseHistory(PulseHistory* hist1, double delay, 
			   PulseHistory* hist2)
{

  /* this block takes care of various possible cases:
   * 1: a schedule with a single item being a pulse has no hist2
   * 2: all calcSchedules which are sub-schedule items will be collapsed
   *    with the calcSchedule they point to, which never has a hist1 or delay
   */
  int hist1Lvls=0, hist2Lvls=0, delayLvl=0;
  if (hist1 != NULL)
    hist1Lvls = hist1->nLevels;
  if (hist2 != NULL)
    hist2Lvls = hist2->nLevels;
  if (delay>0)
    delayLvl = 1;

  nLevels = hist1Lvls + hist2Lvls + delayLvl;
  nPulse = new int[nLevels];
  memCheck(nPulse,"PulseHistory::PulseHistory(...) 'merge' constructor: nPulse");
  td = new double[nLevels];
  memCheck(td,"PulseHistory::PulseHistory(...) 'merge' constructor: td");

  int lvlNum, lvl2Num;

  for (lvlNum=0;lvlNum<hist1Lvls;lvlNum++)
    {
      nPulse[lvlNum] = hist1->nPulse[lvlNum];
      td[lvlNum] = hist1->td[lvlNum];
    }

  if (delayLvl > 0)
    {
      nPulse[lvlNum] = 1;
      td[lvlNum++] = delay;
    }

  for (lvl2Num=0;lvl2Num<hist2Lvls;lvl2Num++)
    {
      nPulse[lvlNum] = hist2->nPulse[lvl2Num];
      td[lvlNum++] = hist2->td[lvl2Num];
    }

  D = NULL;
  if (nLevels>0)
    {
      D = new Matrix[nLevels];
      memCheck(D,"PulseHistory::PulseHistory(...) 'merge' constructor: D");
    }      
    
  setCode = hist1->setCode;
}

/** The correct implementation of this operator must ensure
    that previously allocated space is returned to the free
    store before allocating new space into which to copy the
    object. */
PulseHistory& PulseHistory::operator=(const PulseHistory &p)
{
  if (this == &p)
    return *this;

  int lvlNum;

  nLevels = p.nLevels;
  setCode = p.setCode;

  delete nPulse;
  delete td;
  delete [] D;

  nPulse = NULL;
  td = NULL;
  D = NULL;

  if (nLevels>0)
    {
      nPulse = new int[nLevels];
      memCheck(nPulse,"PulseHistory::operator=(...): nPulse");
      td = new double[nLevels];
      memCheck(td,"PulseHistory::operator=(...): td");
      D = new Matrix[nLevels];
      memCheck(D,"PulseHistory::operator=(...): D");
      
      for (lvlNum=0;lvlNum<p.nLevels;lvlNum++)
	{
	  nPulse[lvlNum] = p.nPulse[lvlNum];
	  td[lvlNum] = p.td[lvlNum];
	  D[lvlNum] = p.D[lvlNum];
	}

    }

  return *this;

}
/****************************
 ********* Solution *********
 ***************************/
/** If not, loops through the matrices and solves them for this
    chain. */
void PulseHistory::setDecay(Chain* chain)
{
  int levelNum;

  if (setCode != chainCode)
    {
      for (levelNum=0;levelNum<nLevels;levelNum++)
	chain->setDecay(D[levelNum],td[levelNum]);
      
      setCode = chainCode;
    }
}

/** It consists of successively multiplying the current transfer
    matrix by the current decay matrix, raising the product to the
    appropriate power, and multiplying once more by the current
    transfer matrix to get the new transfer matrix. */
Matrix PulseHistory::doHistory(Matrix opT)
{
  int levelNum;
  Matrix workT;

  for (levelNum=0;levelNum<nLevels;levelNum++)
    {
      workT = opT * D[levelNum];
      workT = workT ^ (nPulse[levelNum]-1);
      opT = workT * opT;
    }

  return opT;

}

void PulseHistory::write_ph() const
{
    cout << "pulse_history: '" << histName << "" << endl;
    cout << "num_pulsing_levels: " << nLevels << endl;
    cout << "num_pulses_per_level: [" << nPulse[0];
    for (int lvlNum = 1; lvlNum < nLevels; lvlNum++)
    {
      cout << ", " << nPulse[lvlNum];
    }
    cout << "]" << endl;

    cout << "delay_seconds_per_level: [" << td[0];
    for (int lvlNum = 1; lvlNum < nLevels; lvlNum++)
    {
      cout << ", " << td[lvlNum];
    }
    cout << "]" << endl;
}

