/* $Id: PulseHistory.h,v 1.2 1999-08-24 22:06:14 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class stores the information about a pulsing history in the
problem.  Each History object defined in the input will create a
PulseHistory object for use in the solution phase.  These objects will
be accessed through pointers in the calcSchedule objects which define
the whole schedule hierarchy.

 *** Class Members ***
 
 setCode : int
    This is used to keep track of which chain is being solved.  Since
    a pulsing history can be referenced from many places in the
    schedule, this prevents the code from solving the decay matrices
    more than once per chain. (Note that decay matrices are not flux
    dependent.)

 nLevels : int
    The number of levels in this pulsing history.

 nPulse : int*
    The number of pulses in each of the levels of the pulsing history.

 td : double*
    The dwell time between the pulses in each of the levels of the
    pulsing history.

 D : Matrix*
    An array of Matrix objects used to store the decay matrices for
    this pulsing history, one per dwell time, i.e. level.

 *** Member Functions ***

 * - Constructors & Destructors - *

 PulseHistory (int, int*, double*);
    When called with no arguments, the default constructor sets the
    'nLevels' to 0 and 'nPulse' and 'td' to NULL.  Otherwise, they are
    set with the arguments.  Note that the pointers are copied, and
    not the arrays themselves.  In both cases, 'setCode' is
    initialized to -1.

 PulseHistory (const PulseHistory&)
    The copy constructor copies all members including an
    element-by-element copies of the two arrays.

 PulseHistory (PulseHistory*, double, PulseHistory*) 

    This constructor acts to combine two pulsing histories into one.
    This action is valid (and invoked) whenever a schedule has a
    single sub-schedule.  Basically, a pulsing history with N levels,
    followed by a delay of time D, all pulsed with a history of M
    levels is the same as a pulsing history with N+M+1 levels where
    level N+1 has one pulse and delay time D.  It is possible for N,M
    and/or D to be zero (N+M+D is always > 0), and these cases are
    taken care of.

 ~PulseHistory ()
    Inline destructor simply deletes all storage for objects of class
    PulseHistory.

 PulseHistory& operator=(const PulseHistory&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.

 * - Solution - *

 void setDecay(Chain*)
    This function checks to see if the decay matrices for the chain
    pointed to by the argument have already been solved, and if not,
    loops through the matrices and solves them for this chain.

 Matrix doHistory(Matrix)
    This function performs the mathematics required to represent the
    pulsing history.  It consists of successively multiplying the
    current transfer matrix by the current decay matrix, raising the
    product to the appropriate power, and multiplying once more by the
    current transfer matrix to get the new transfer matrix.

 */

#ifndef _PULSEHISTORY_H
#define _PULSEHISTORY_H

#include "Util/Matrix.h"

class PulseHistory
{

protected:
  int setCode, nLevels, *nPulse;
  double *td;
  Matrix *D;

public:
  /* Service */
  PulseHistory (int nlvls=0, int *pulse = NULL, double *decay=NULL );
  PulseHistory (const PulseHistory&);
  PulseHistory (PulseHistory*, double, PulseHistory*);
  ~PulseHistory ()
    { delete td; delete nPulse; delete [] D; };

  PulseHistory& operator=(const PulseHistory&);

  /* Solution */
  void setDecay(Chain*);
  Matrix doHistory(Matrix);

};

#endif
