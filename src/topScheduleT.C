/* $Id: topScheduleT.C,v 1.5 2003-01-13 04:34:30 fateneja Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Tally: functions directly lated to tallying results
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "topScheduleT.h"
#include "topSchedule.h"

#include "NuclearData.h"

/****************************
 ********* Service **********
 ***************************/

int topScheduleT::nCoolingTimes = 0;

/** This constructor invokes the equivalent base class constructor
    calcScheduleT(calcSchedule*).  In addition, if no argument is
    given, it sets 'coolT' to NULL, otherwise, it creates and
    initializes storage for 'nCoolingTimes' Matrix objects.  See
    calcSchedule(calcSchedule*) for more information. */
topScheduleT::topScheduleT(topSchedule *top) :
  calcScheduleT(top)
{
  coolT = NULL;

  if (nCoolingTimes > 0)
    {
      coolT = new Matrix[nCoolingTimes];
      memCheck(coolT,"topScheduleT::topScheduleT(...) constructor: coolT");
    }

}

/** This constructor invokes the equivalent base class constructor
    calcScheduleT(const calcScheduleT&).  If the number of
    cooling times is defined, it then copies each of the individual
    transfer matrices.  As such, it recursively copies the transfer
    matrices of an entire schedule. */
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

/** This assignment operator behaves similarly to the copy
    constructor, but instead of calling the base class constructor, it
    as code copied from that class' assignment operator.  As with all
    assignment operators in ALARA, care must be taken to free dynamic
    memory before creating new memory for copying into. */
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

topScheduleT::~topScheduleT()
{
  delete[] coolT;
}

/****************************
 ********** Tally ***********
 ***************************/

double* topScheduleT::results(int rank)
{
  int idx, coolNum;

  double *data = new double[nCoolingTimes+1];

  switch(NuclearData::getMode())
    {
    case MODE_FORWARD:
      idx = rank*(rank+1)/2;
      break;
    case MODE_REVERSE:
      idx = totalT.getSize();
      idx = (idx*(idx+1))/2-1;
      idx -= rank;
      break;
    }

  data[0] = totalT[idx];

  for (coolNum=0;coolNum<nCoolingTimes;coolNum++)
    data[coolNum+1] = coolT[coolNum][idx];

  return data;
}
