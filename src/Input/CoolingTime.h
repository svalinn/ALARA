/* $Id: CoolingTime.h,v 1.4 2002-08-05 20:23:15 fateneja Exp $ */
#include "alara.h"

#ifndef _COOLINGTIME_H
#define _COOLINGTIME_H

/* cooling time head */
#define COOL_HEAD -1

/** \brief This class is a linked list of the input information for 
 *         after-shutdown cooling times.
 *   
 *  The single object of class Input has a list of cooling times.
 *  The first element in each list has type COOL_HEAD (defined through
 *  the 'coolingTime' member), and contains no problem data.
 */

class CoolingTime
{
protected:
  char 
    /// This indicates the time units specified in the input file.
    units;

  double 
    /// The value of this cooling time in the units defined in 'units'.
    coolingTime;
  
  CoolingTime* 
    /// This is the pointer to the next cooling time.
    next;

public:
  /// Default constructor
  /** Default constructor creates a blank list head with no arguments.
      Otherwise, it sets both the time and units and initializes the
      'next' pointer to NULL. */
  CoolingTime(double coolTime=COOL_HEAD, char unts=' ');

  /// Copy constructor
  /** Copy constructor copies scalar members and sets 'next' to NULL. */
  CoolingTime(const CoolingTime&);

  /// Inline destructor destroys the whole list by deleting the 'next'.
  ~CoolingTime()
    { delete next; };

  /// This function reads a whole list of cooling times from the input file,
  /// attached to the stream given as an argument.
  /** It reads a list of times and units until it finds the keyword "end".
      It is called through the head of the cooling times list. */
  void getCoolingTimes(istream&);

  /// This function makes an array of cooling times, all converted to
  /// seconds.
  /** Called through the head of the cooling times list, it
      allocates the correct number of doubles, assigning them to the pointer
      reference argument.  It returns the number of cooling times. */
  int makeCoolingTimes(double *&);

  /// This function writes a header for the standard response table.
  /** There is a column for the isotope, a column for the @shutdown 
      result, and then a column for each after-shutdown cooling time. */
  void writeHeader();

  /// This function writes a header for the table of totals. 
  /** There is a  column indicating the counter for the total in question,
      one column for @ shutdown results, and then one column for each of the
      after-shutdown cooling times. */
  void writeTotalHeader(char*);

  /// This function writes an appropriately sized separator of
  /// "===...===" to frame the table.
  void writeSeparator();

  /// Inline function to return the boolean result indicating whether or
  /// not this CoolingTime is the head of the list. 
  int head() { return (coolingTime == COOL_HEAD);};
};


#endif
