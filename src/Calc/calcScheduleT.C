/* File sections:
 * Service: constructors, destructors
 * Preproc: functions directly related to preprocessing of input
 *          prior to solution 
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "calcScheduleT.h"
#include "calcSchedule.h"

calcScheduleT::calcScheduleT(calcSchedule *sched) :
  nItems(0), subSchedT(NULL)
{
  if (sched != NULL && sched->numItems() > 0)
    {
      nItems = sched->numItems();

      subSchedT = new calcScheduleT*[nItems];
      memCheck(subSchedT, "calcScheduleT::calcScheduleT(...) constructor: subSchedT");

      for (int itemNum=0;itemNum<nItems;itemNum++)
	{
	  subSchedT[itemNum] = new calcScheduleT( (*sched)[itemNum] );
	  memCheck(subSchedT[itemNum],
		   "calcScheduleT::calcScheduleT(...) constructor: subSchedT[itemNum]");
	}
    }

}

calcScheduleT::calcScheduleT(const calcScheduleT& c) :
  totalT(c.totalT), histT(c.histT), opBlockT(c.opBlockT), nItems(c.nItems)
{
  if (nItems > 0)
    {
      subSchedT = new calcScheduleT*[nItems];
      memCheck(subSchedT,"calcScheduleT::calcScheduleT(...) copy constructor: subSchedT");
      
      for (int itemNum=0;itemNum<nItems;itemNum++)
	{
	  subSchedT[itemNum] = new calcScheduleT( *(c.subSchedT[itemNum]) );
	  memCheck(subSchedT[itemNum],
		   "calcScheduleT::calcScheduleT(...) copy constructor: subSchedT[itemNum]");
	}
    }
}
      
calcScheduleT& calcScheduleT::operator=(const calcScheduleT& c)
{
  if (this == &c)
    return *this;

  totalT = c.totalT;
  histT = c.histT;
  opBlockT = c.opBlockT;
  nItems = c.nItems;

  delete [] subSchedT;
  subSchedT = NULL;

  if (nItems > 0)
    {
      subSchedT = new calcScheduleT*[nItems];
      memCheck(subSchedT,"calcScheduleT::operator=(...): subSchedT");
      
      for (int itemNum=0;itemNum<nItems;itemNum++)
	{
	  subSchedT[itemNum] = new calcScheduleT( *(c.subSchedT[itemNum]) );
	  memCheck(subSchedT[itemNum],"calcScheduleT::operator=(...): subSchedT[itemNum]");
	}
    }

  return *this;

}
      


