/* $Id: topScheduleT.h,v 1.3 2000-01-17 18:45:21 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is the head of a linked hierarchy of storage for the
transfer matrices throughout the schedule.  This class is derived from
calcScheduleT.  See its description for more details, particularly for
inherited members and methods.

 *** Static Class Members ***

 nCoolingtimes : int
    The number of cooling times for which result matrices need to be
    stored.

 *** Class Members ***

 coolT : Matrix*
    An array of Matrices for storing the results of the schedule
    following the after-shutdown cooling times.

 *** Static Member Functions ***

 * - Preproc - *

 void setNumCoolingTimes(int)
    Inline function providing access to set static member
    'nCoolingTimes'.

 void getNumCoolingTimes()
    Inline function providing read access to static member
    'nCoolingTimes'.

 *** Member Functions ***
 
 * - Constructors & Destructors - *

 topScheduleT(topSchedule*)
    This constructor invokes the equivalent base class constructor
    calcScheduleT(calcSchedule*).  In addition, if no argument is
    given, it sets 'coolT' to NULL, otherwise, it creates and
    initializes storage for 'nCoolingTimes' Matrix objects.  See
    calcSchedule(calcSchedule*) for more information.

 topScheduleT(const topScheduleT&)
    This copy constructor invokes the equivalent base class
    constructor calcScheduleT(const calcScheduleT&).  If the number of
    coolig times is defined, it then copies each of the individual
    transfer matrices.  As such, it recursively copies the transfer
    matrices of an entire schedule.

 topScheduleT& operator=(const topScheduleT&);
    This assignment operator behaves similarly to the copy
    constructor, but instead of calling the base class constructor, it
    as code copied from that class' assignment operator.  As with all
    assignment operators in ALARA, care must be taken to free dynamic
    memory before creating new memory for copying into.

 * - Tally - *

 double* results(int)
    This function returns an array or results for the rank specified
    by the first argument, with one member of the array for each of
    the shutdown time and the various after-shutdown cooling times.

 * - Utility - *

 Matrix& cool(int)
    This funtion returns a reference to the Matrix class member of
    array 'coolT' indicated by the argument.  With this function, we
    can avoid having friend classes and functions.

 */

#ifndef _TOPSCHEDULET_H
#define _TOPSCHEDULET_H

#include "calcScheduleT.h"
#include "Util/Matrix.h"

class topScheduleT : public calcScheduleT
{

protected:
  static int nCoolingTimes;

  Matrix *coolT;

public:
  /* Preproc */
  static void setNumCoolingTimes(int nCool)
    { nCoolingTimes = nCool; };
  static int getNumCoolingTimes()
    { return nCoolingTimes; };

  /* Service */
  topScheduleT(topSchedule *top = NULL);
  topScheduleT(const topScheduleT&);
      
  topScheduleT& operator=(const topScheduleT&);

  /* Tally */
  double *results(int);

  /* Utility */
  Matrix& cool(int idx) { return coolT[idx]; };
};

#endif
