/* $Id: PulseHistory.h,v 1.3 2002-08-05 20:23:11 fateneja Exp $ */
#include "alara.h"

#ifndef _PULSEHISTORY_H
#define _PULSEHISTORY_H

#include "Util/Matrix.h"

/** \brief This class stores the information about a pulsing history 
 *         in the problem.
 *          
 *  Each History object defined in the input will create a PulseHistory
 *  object for use in the solution phase.  These objects will be 
 *  accessed through pointers in the calcSchedule objects which define
 *  the whole schedule hierarchy.
 */

class PulseHistory
{

protected:
  int
    /// This is used to keep track of which chain is being solved.
    /** Since a pulsing history can be referenced from many places 
        in the schedule, this prevents the code from solving the 
        decay matrices more than once per chain. (Note that decay 
        matrices are not flux dependent.) */
    setCode, 
    
    /// The number of levels in this pulsing history.
    nLevels, 
    
    /// The number of pulses in each of the levels of the pulsing 
    /// history.
    *nPulse;

  double
    /// The dwell time between the pulses in each of the levels of 
    /// the pulsing history.
    *td;

  Matrix
    /// An array of Matrix objects used to store the decay matrices 
    /// for this pulsing history, one per dwell time, i.e. level.
    *D;

public:
  /// Default constructor, when called with no arguments
  /** When called with no arguments, the default constructor sets the
      'nLevels' to 0 and 'nPulse' and 'td' to NULL.  Otherwise, they 
      are set with the arguments.  Note that the pointers are copied, 
      and not the arrays themselves.  In both cases, 'setCode' is
      initialized to -1. */
  PulseHistory (int nlvls=0, int *pulse = NULL, double *decay=NULL );

  /// Copy constructor 
  /** Copies all members including an element-by-element copies of 
      the two arrays. */
  PulseHistory (const PulseHistory&);

  /// This constructor acts to combine two pulsing histories into one.
  /** This action is valid (and invoked) whenever a schedule has a
      single sub-schedule.  Basically, a pulsing history with N levels,
      followed by a delay of time D, all pulsed with a history of M
      levels is the same as a pulsing history with N+M+1 levels where
      level N+1 has one pulse and delay time D.  It is possible for N,M
      and/or D to be zero (N+M+D is always > 0), and these cases are
      taken care of. */
  PulseHistory (PulseHistory*, double, PulseHistory*);

  /// Inline destructor simply deletes all storage for objects of 
  /// class PulseHistory.
  ~PulseHistory ()
    { delete td; delete nPulse; delete [] D; };

  /// Overloaded assignment operator
  /** The correct implementation of this operator must ensure 
      that previously allocated space is returned to the free 
      store before allocating new space into which to copy the 
      object. */
  PulseHistory& operator=(const PulseHistory&);

  /// This function checks to see if the decay matrices for the chain
  /// pointed to by the argument have already been solved.
  /** If not, loops through the matrices and solves them for this 
      chain. */
  void setDecay(Chain*);

  /// This function performs the mathematics required to represent the
  /// pulsing history.
  /** It consists of successively multiplying the current transfer
      matrix by the current decay matrix, raising the product to the 
      appropriate power, and multiplying once more by the current 
      transfer matrix to get the new transfer matrix. */
  Matrix doHistory(Matrix);

};

#endif
