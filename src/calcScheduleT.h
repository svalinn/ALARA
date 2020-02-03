/* $Id: calcScheduleT.h,v 1.5 2003-01-13 04:34:29 fateneja Exp $ */
#include "alara.h"

#ifndef CALCSCHEDULET_H
#define CALCSCHEDULET_H

#include "Matrix.h"

/** \brief This class is used to create a linked hierarchy of storage 
 *         for the transfer matrices throughout the schedule.
 *
 *  For each interval, a calcScheduleT hieararchy exists to mirror 
 *  the problem-wide calcSchedule hierarchy.  Please see notes on 
 *  class calcSchedule to for additional understanding.
 */

class calcScheduleT
{
protected:
  /// This is the storage for the transfer matrix representing the 
  /// total of the corresponding calcSchedule, including operation,
  /// pulsing and decay.
  Matrix totalT;
     
  /// This is the storage for the transfer matrix representing the
  /// operation and pulsing of the corresponding calcSchedule.
  Matrix histT;
    
  /// This is the storage for the transfer matrix representing only 
  /// the operation of the corresponding calcSchedule.
  Matrix opBlockT;

  /// The number of sub-schedule storage matrices.
  int nItems;

  /// This array of pointers points to the transfer matrix storage 
  /// which corresponds to the sub-schedules of the corresponding
  /// calcSchedule, 'calcSchedule::subSched'.
  calcScheduleT **subSchedT;

public:
  /// Default constructor, when called with no arguments
  calcScheduleT(calcSchedule *sched=NULL);

  /// Copy constructor
  calcScheduleT(const calcScheduleT&);

  /// Inline destructor deletes 'subSchedT' in an array sense, 
  /// therefore deleting an entire hierarchy in a single invocation.
  ~calcScheduleT()
    { while (nItems-->0) { delete subSchedT[nItems];} delete subSchedT; };
   
  /// Overload assignment operator
  calcScheduleT& operator=(const calcScheduleT&);

  /// Allows access to opBlockT
  Matrix& opBlock() { return opBlockT;};

  /// Allows access to histT
  Matrix& hist() { return histT;};

  /// Allows access to totalT
  Matrix& total() { return totalT;};
  
  /// Inline operator function provides C style indexing of 
  /// subschedule matrices.
  calcScheduleT*& operator[] (int idx) { return subSchedT[idx]; } ;
};

#endif



