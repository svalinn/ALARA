/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "PulseHistory.h"

#include "Chains/Chain.h"

/****************************
 ********* Service **********
 ***************************/


PulseHistory::PulseHistory(int nlvls, int *pulse, double *decay) :
  setCode(-1), nLevels(nlvls),  nPulse(pulse),  td(decay)
{

  D = NULL;
  if (nLevels>0)
    {
      D = new Matrix[nLevels];
      memCheck(D,"PulseHistory::PulseHistory(...) constructor: D");
    }


}

PulseHistory::PulseHistory(const PulseHistory &p) : 
  setCode(p.setCode),nLevels(p.nLevels)
{
  int lvlNum;

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
    
}

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

