/* $Id: topSchedule.h,v 1.4 2003-01-13 04:34:30 fateneja Exp $ */
#include "alara.h"

#ifndef TOPSCHEDULE_H
#define TOPSCHEDULE_H

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
  /// The number of after-shutdown cooling times at which a 
  /// solution should be calculated. 
  int nCoolingTimes;

  /// An array of after-shutdown cooling times in seconds.
  double *coolingTime;

  /// An array of storage for the decay matrices associated with the
  /// after-shutdown cooling times.
  Matrix *coolD;

public:
  /// Inline constructor invokes base class copy constructor and
  /// initializes 'coolD' to NULL.
  topSchedule(calcSchedule *sched,CoolingTime* coolList=NULL);

  /// Copy constructor
  topSchedule(const topSchedule&);

  /// This standard destructor simply deletes the storage for 'coolD'
  /// and 'coolingTime'.
  ~topSchedule();
  
  /// Overloaded assignment operator
  topSchedule& operator=(const topSchedule&);

  /// This function serves a similar purpose to
  /// calcSchedule::setDecay(...).
  void setDecay(Chain*);

  /// This function serves a similar purpose to calcSchedule::setT(...).
  void setT(Chain*, topScheduleT*);
};

#endif
