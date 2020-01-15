/* $Id: topScheduleT.h,v 1.5 2003-01-13 04:34:30 fateneja Exp $ */
#include "alara.h"

#ifndef TOPSCHEDULET_H
#define TOPSCHEDULET_H

#include "calcScheduleT.h"
#include "Util/Matrix.h"

/** \brief This class is the head of a linked hierarchy of storage for
 *         the transfer matrices throughout the schedule.
 *
 *  This class is derived from calcScheduleT.  See its description for
 *  more details, particularly for inherited members and methods.
 */

class topScheduleT : public calcScheduleT
{

protected:
  /// The number of cooling times for which result matrices need to 
  /// be stored.
  static int nCoolingTimes;

  /// An array of Matrices for storing the results of the schedule
  /// following the after-shutdown cooling times.
  Matrix *coolT;

public:
  /// Inline function providing access to set static member
  /// 'nCoolingTimes'.
  static void setNumCoolingTimes(int nCool)
    { nCoolingTimes = nCool; };

  /// Inline function providing read access to static member
  /// 'nCoolingTimes'.
  static int getNumCoolingTimes()
    { return nCoolingTimes; };

  
  /// Default constructor
  topScheduleT(topSchedule *top = NULL);

  /// Copy Constructor
  topScheduleT(const topScheduleT&);
   
  /// Overlaoded assignment operator
  topScheduleT& operator=(const topScheduleT&);

  /// This function returns an array or results for the rank specified
  /// by the first argument, with one member of the array for each of
  /// the shutdown time and the various after-shutdown cooling times.
  double *results(int);


  /// This funtion returns a reference to the Matrix class member of
  /// array 'coolT' indicated by the argument.
  /** With this function, we can avoid having friend classes and
      functions. */
  Matrix& cool(int idx) { return coolT[idx]; };
};

#endif
