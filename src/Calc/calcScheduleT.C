/* $Id: calcScheduleT.C,v 1.4 2003-01-13 04:34:29 fateneja Exp $ */
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

/** When called with no argumetns it initializes all the matrices
    to identity matrices of size 0 and sets 'subSchedT' to NULL.
    Otherwise, 'subSchedT' is initialized to store the appropriate
    number ('calcSchedule::nItems') of pointers and these pointers
    are filled with new calcScheduleT objects to continue the
    hierarchy.  The entire hierarchy is created recursively with
    one outside call through a topScheduleT derived class
    object. */
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

/** This constructor copies each of the members, and allocates
    new space AND new object for each of the sub-matrices.  That is,
    a single invocation of this operator copies the entire hierarchy
    of calcScheduleT objects. */
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

/** The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that it
    also copies the 'subSchedT' objects (NOT THE POINTER) effectively
    copying a whole hierarchy with one invocation. */  
calcScheduleT& calcScheduleT::operator=(const calcScheduleT& c)
{
  if (this == &c)
    return *this;

  totalT = c.totalT;
  histT = c.histT;
  opBlockT = c.opBlockT;

  while (nItems-->0)
    delete subSchedT[nItems];
  delete subSchedT;
  subSchedT = NULL;

  nItems = c.nItems;

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
      


