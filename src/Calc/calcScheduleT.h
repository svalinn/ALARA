#include "alara.h"

/* ******* Class Description ************

This class is used to create a linked hierarchy of storage for the
transfer matrices throughout the schedule.  For each interval, a
calcScheduleT hieararchy exists to mirror the problem-wide
calcSchedule hierarchy.  Please see notes on class calcSchedule to
for additional understanding.

 *** Class Members ***

 totalT : Matrix
    This is the storage for the transfer matrix representing the total
    of the corresponding calcSchedule, including operation, pulsing and
    decay.

 histT : Matrix
    This is the storage for the transfer matrix representing the
    operation and pulsing of the corresponding calcSchedule.

 opBlockT : Matrix
    This is the storage for the transfer matrix representing only the
    operation of the corresponding calcSchedule.

 nItems : int
    The number of sub-schedule storage matrices.

 subSchedT : calcScheduleT**
    This array of pointers points to the transfer matrix storage which
    corresponds to the sub-schedules of the corresponding
    calcSchedule, 'calcSchedule::subSched'.

 *** Member Functions ***

 * - Constructors and Destructors - *

 calcScheduleT(calcSchedule*);
    When called with no arguments, the default constructor initializes
    all the matrices to identity matrices of size 0 and sets
    'subSchedT' to NULL.  Otherwise, 'subSchedT' is initialized to
    store the appropriate number ('calcSchedule::nItems') of pointers
    and these pointers are filled with new calcScheduleT objects to
    continue the hierarchy.  The entire hierarchy is created
    recursively with one outside call through a topScheduleT derived
    class object.

 calcScheduleT(const calcScheduleT&)
    This copy constructor copies each of the members, and allocates
    new space AND new object for each of the sub-matrices.  That is, a
    single invocation of this operator copies the entire hierarchy of
    calcScheduleT objects.

 ~calcScheduleT()
    Inline destructor deletes 'subSchedT' in an array sense, therefore
    deleting an entire hierarchy in a single invocation.

 calcScheduleT& operator=(const calcScheduleT&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that it
    also copies the 'subSchedT' objects (NOT THE POINTER) effectively
    copying a whole hierarchy with one invocation.

 * - Access - *

 By providing access to the members of calcScheduleT, these functions
 prevent the necessity for friends to this class.

 Matrix& opBlock() { return opBlockT;};
 Matrix& hist() { return histT;};
 Matrix& total() { return totalT;};

 calcScheduleT*& operator[] (int idx) { return subSchedT[idx]; } ; 

*/

#ifndef _CALCSCHEDULET_H
#define _CALCSCHEDULET_H

#include "Util/Matrix.h"

class calcScheduleT
{
protected:
  Matrix totalT, histT, opBlockT;
  int nItems;
  calcScheduleT **subSchedT;

public:
  /* Service */
  calcScheduleT(calcSchedule *sched=NULL);
  calcScheduleT(const calcScheduleT&);
  ~calcScheduleT()
    { while (nItems-->0) { delete subSchedT[nItems];} delete subSchedT; };

  calcScheduleT& operator=(const calcScheduleT&);

  Matrix& opBlock() { return opBlockT;};
  Matrix& hist() { return histT;};
  Matrix& total() { return totalT;};

  calcScheduleT*& operator[] (int idx) { return subSchedT[idx]; } ;

};

#endif
