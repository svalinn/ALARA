/* $Id: topScheduleT.h,v 1.4 2002-08-05 20:23:13 fateneja Exp $ */
#include "alara.h"

#ifndef _TOPSCHEDULET_H
#define _TOPSCHEDULET_H

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
  static int 
    /// The number of cooling times for which result matrices need to 
    /// be stored.
    nCoolingTimes;

  Matrix 
    /// An array of Matrices for storing the results of the schedule
    /// following the after-shutdown cooling times.
    *coolT;

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
  /** This constructor invokes the equivalent base class constructor
      calcScheduleT(calcSchedule*).  In addition, if no argument is
      given, it sets 'coolT' to NULL, otherwise, it creates and
      initializes storage for 'nCoolingTimes' Matrix objects.  See
      calcSchedule(calcSchedule*) for more information. */
  topScheduleT(topSchedule *top = NULL);

  /// Copy Constructor
  /** This constructor invokes the equivalent base class constructor 
      calcScheduleT(const calcScheduleT&).  If the number of
      cooling times is defined, it then copies each of the individual
      transfer matrices.  As such, it recursively copies the transfer
      matrices of an entire schedule. */
  topScheduleT(const topScheduleT&);
   
  /// Overlaoded assignment operator
  /** This assignment operator behaves similarly to the copy
      constructor, but instead of calling the base class constructor, it
      as code copied from that class' assignment operator.  As with all
      assignment operators in ALARA, care must be taken to free dynamic
      memory before creating new memory for copying into. */
  topScheduleT& operator=(const topScheduleT&);

  double 
    /// This function returns an array or results for the rank specified
    /// by the first argument, with one member of the array for each of
    /// the shutdown time and the various after-shutdown cooling times.
    *results(int);


  /// This funtion returns a reference to the Matrix class member of
  /// array 'coolT' indicated by the argument.
  /** With this function, we can avoid having friend classes and
      functions. */
  Matrix& cool(int idx) { return coolT[idx]; };
};

#endif
