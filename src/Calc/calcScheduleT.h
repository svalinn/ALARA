/* $Id: calcScheduleT.h,v 1.4 2002-08-05 20:23:12 fateneja Exp $ */
#include "alara.h"

#ifndef _CALCSCHEDULET_H
#define _CALCSCHEDULET_H

#include "Util/Matrix.h"

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
  Matrix 
    /// This is the storage for the transfer matrix representing the 
    /// total of the corresponding calcSchedule, including operation,
    /// pulsing and decay.
    totalT, 
     
    /// This is the storage for the transfer matrix representing the
    /// operation and pulsing of the corresponding calcSchedule.
    histT, 
    
    /// This is the storage for the transfer matrix representing only 
    /// the operation of the corresponding calcSchedule.
    opBlockT;

  int
    /// The number of sub-schedule storage matrices.
    nItems;


  calcScheduleT
    /// This array of pointers points to the transfer matrix storage 
    /// which corresponds to the sub-schedules of the corresponding
    /// calcSchedule, 'calcSchedule::subSched'.
    **subSchedT;

public:
  /* Service */
  /// Default constructor, when called with no arguments
  /** When called with no argumetns it initializes all the matrices
      to identity matrices of size 0 and sets 'subSchedT' to NULL.
      Otherwise, 'subSchedT' is initialized to store the appropriate
      number ('calcSchedule::nItems') of pointers and these pointers
      are filled with new calcScheduleT objects to continue the 
      hierarchy.  The entire hierarchy is created recursively with 
      one outside call through a topScheduleT derived class 
      object. */
  calcScheduleT(calcSchedule *sched=NULL);

  /// Copy constructor
  /** This constructor copies each of the members, and allocates
      new space AND new object for each of the sub-matrices.  That is, 
      a single invocation of this operator copies the entire hierarchy 
      of calcScheduleT objects. */
  calcScheduleT(const calcScheduleT&);

  /// Inline destructor deletes 'subSchedT' in an array sense, 
  /// therefore deleting an entire hierarchy in a single invocation.
  ~calcScheduleT()
    { while (nItems-->0) { delete subSchedT[nItems];} delete subSchedT; };

   
  /// Overload assignment operator
  /** The correct implementation of this operator must ensure that
      previously allocated space is returned to the free store before
      allocating new space into which to copy the object. Note that it
      also copies the 'subSchedT' objects (NOT THE POINTER) effectively
      copying a whole hierarchy with one invocation. */
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



