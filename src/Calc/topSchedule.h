#include "alara.h"

/* ******* Class Description ************

This class is used as the head of a linked hierarchy of schedule
information.  This class is derived publicly from class calcSchedule.
Read the description of that class for important information on the
calcSchedule hierarchy creation and the inherited functions of this
class.

 *** Static Class Members ***

 nCoolingTimes : int
    The number of after-shutdown cooling times at which a solution
    should be calculated.

 coolingTime : double*
   An array of after-shutdown cooling times in seconds.

 coolD : Matrix*
   An array of storage for the decay matrices associated with the
   after-shutdown cooling times.

 *** Member Functions ***

 * - Constructors & Destructors - *

 topSchedule(calcSchedule*,CoolingTime*) 
    Inline constructor invokes base class copy constructor and
    initializes 'coolD' to NULL .  This is used to convert a
    calcSchedule object, pointed to by the first argument, to a
    topSchedule object.  At the same time, this function is
    initializes the number of after-shutdown cooling times of
    interest.  The second argument is the head of the list of input
    coolingTimes which is parsed to set the local member.  After
    establishing the number of cooling times, space is allocated and
    initialized for 'coolD'.

 topSchedule(const topSchedule&)
    This copy constructor first invokes its base class copy
    constructor.  It then copies the number of cooling times, copying
    'coolD' and 'coolingTime' on an element-by-element basis if it is
    greater than 0.

 ~topSchedule()
    This standard destructor simply deletes the storage for 'coolD'
    and 'coolingTime'.

 topSchedule operator=(const topSchedule&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.

 * - Solution - *

 void setDecay(Chain*)

    This function serves a similar purpose to
    calcSchedule::setDecay(...).  It does not set a decay matrix for
    the delay since no delay is applied to a topSchedule.  It checks
    for the existence of a history before calling setDecay on it,
    since a topSchedule may not have a pulsing history.  It calls
    Chain::setDecay for each of the 'coolD' matrices.

 void setT(Chain*, topScheduleT*) 
    This function serves a similar purpose to calcSchedule::setT(...).
    It does not apply any final decay operation.  It checks for the
    existence of the pulsing history before applying it.  It applies
    decays for each of the after-shutdown cooling times, storing them
    in the special matrices of the topScheduleT object passed in the
    second argument.

 */


#ifndef _TOPSCHEDULE_H
#define _TOPSCHEDULE_H

#include "calcSchedule.h"

class topSchedule : public calcSchedule
{
protected:
  int nCoolingTimes;
  double *coolingTime;
  Matrix *coolD;

public:
  /* Service */
  topSchedule(calcSchedule *sched,CoolingTime* coolList=NULL);
  topSchedule(const topSchedule&);
  ~topSchedule();
  
  topSchedule& operator=(const topSchedule&);

  /* Solution */
  void setDecay(Chain*);
  void setT(Chain*, topScheduleT*);
};

#endif
