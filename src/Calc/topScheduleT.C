/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Tally: functions directly lated to tallying results
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "topScheduleT.h"
#include "topSchedule.h"

/****************************
 ********* Service **********
 ***************************/

int topScheduleT::nCoolingTimes = 0;

topScheduleT::topScheduleT(topSchedule *top) :
  calcScheduleT(top)
{
  coolT = NULL;

  if (nCoolingTimes > 0)
    {
      coolT = new Matrix[nCoolingTimes];
      memCheck(coolT,"topScheduleT::topScheduleT(...) constructor: coolT");
    }

};
  
topScheduleT::topScheduleT(const topScheduleT &t) :
  calcScheduleT(t)
{
  coolT = NULL;

  if (nCoolingTimes > 0)
    {
      coolT = new Matrix[nCoolingTimes];
      memCheck(coolT,"topScheduleT::topScheduleT(...) constructor: coolT");

      for (int coolNum=0;coolNum<nCoolingTimes;coolNum++)
	coolT[coolNum] = t.coolT[coolNum];
    }
}

topScheduleT& topScheduleT::operator=(const topScheduleT &t)
{

  if (this == &t)
    return *this;

  /**** BEGIN copied directly from calcSchedule.C ***/
  totalT = t.totalT;
  histT = t.histT;
  opBlockT = t.opBlockT;

  while (nItems-->0)
    delete subSchedT[nItems];
  delete subSchedT;
  subSchedT = NULL;

  nItems = t.nItems;

  if (nItems > 0)
    {
      subSchedT = new calcScheduleT*[nItems];
      memCheck(subSchedT,"calcScheduleT::operator=(...): subSchedT");
      
      for (int itemNum=0;itemNum<nItems;itemNum++)
	{
	  subSchedT[itemNum] = new calcScheduleT( *(t.subSchedT[itemNum]) );
	  memCheck(subSchedT[itemNum],"calcScheduleT::operator=(...): subSchedT[itemNum]");
	}
    }
  /**** END copied directly from calcSchedule.C ***/

  delete [] coolT;
  coolT = NULL;

  if (nCoolingTimes > 0)
    {
      coolT = new Matrix[nCoolingTimes];
      memCheck(coolT,"topScheduleT::topScheduleT(...) constructor: coolT");

      for (int coolNum=0;coolNum<nCoolingTimes;coolNum++)
	coolT[coolNum] = t.coolT[coolNum];
    }

  return *this;

}

/****************************
 ********** Tally ***********
 ***************************/

double* topScheduleT::results(int rank)
{
  int idx, coolNum;

  double *data = new double[nCoolingTimes+1];
  
  idx = rank*(rank+1)/2;

  data[0] = totalT[idx];

  for (coolNum=0;coolNum<nCoolingTimes;coolNum++)
    data[coolNum+1] = coolT[coolNum][idx];

  return data;
}
