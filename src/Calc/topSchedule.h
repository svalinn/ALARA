/* $Id: topSchedule.h,v 1.3 2002-08-05 20:23:12 fateneja Exp $ */
#include "alara.h"

#ifndef _TOPSCHEDULE_H
#define _TOPSCHEDULE_H

#include "calcSchedule.h"

/** \brief This class is used as the head of a linked hierarchy of 
 *         schedule information.
 *  
 *  This class is derived publicly from class calcSchedule. Read
 *  the description of that class for important information on the
 *  calcSchedule hierarchy creation and the inherited functions of 
 *  this class.
 */

class topSchedule : public calcSchedule
{
protected:
  int 
    /// The number of after-shutdown cooling times at which a 
    /// solution should be calculated. 
    nCoolingTimes;

  double
    /// An array of after-shutdown cooling times in seconds.
    *coolingTime;

  Matrix
    /// An array of storage for the decay matrices associated with the
    /// after-shutdown cooling times.
    *coolD;

public:
  /// Inline constructor invokes base class copy constructor and
  /// initializes 'coolD' to NULL.
  /** This is used to convert a calcSchedule object, pointed to by 
      the first argument, to a topSchedule object.  At the same time, 
      this function is initializes the number of after-shutdown 
      cooling times of interest.  The second argument is the head of 
      the list of input coolingTimes which is parsed to set the local 
      member.  After establishing the number of cooling times, space 
      is allocated and initialized for 'coolD'. */
  topSchedule(calcSchedule *sched,CoolingTime* coolList=NULL);

  /// Copy constructor
  /** This constructor first invokes its base class copy constructor.
      It then copies the number of cooling times, copying 'coolD' and
      'coolingTime' on an element-by-element basis if it is greater
      than 0. */
  topSchedule(const topSchedule&);

  /// This standard destructor simply deletes the storage for 'coolD'
  /// and 'coolingTime'.
  ~topSchedule();
  
   /// Overloaded assignment operator
   /** The correct implementation of this operator must ensure that
       previously allocated space is returned to the free store before
       allocating new space into which to copy the object. */
  topSchedule& operator=(const topSchedule&);

   /// This function serves a similar purpose to
   /// calcSchedule::setDecay(...).
   /** It does not set a decay matrix for the delay since no delay is
       applied to a topSchedule.  It checks for the existence of a 
       history before calling setDecay on it, since a topSchedule may 
       not have a pulsing history.  It calls Chain::setDecay for each of 
       the 'coolD' matrices. */
  void setDecay(Chain*);

  /// This function serves a similar purpose to calcSchedule::setT(...).
  /** It does not apply any final decay operation.  It checks for the
      existence of the pulsing history before applying it.  It applies
      decays for each of the after-shutdown cooling times, storing them
      in the special matrices of the topScheduleT object passed in the
      second argument. */
  void setT(Chain*, topScheduleT*);
};

#endif
