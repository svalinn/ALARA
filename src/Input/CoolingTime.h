/* $Id: CoolingTime.h,v 1.5 2003-01-13 04:34:55 fateneja Exp $ */
#include "alara.h"

#ifndef COOLINGTIME_H
#define COOLINGTIME_H

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
  /// This indicates the time units specified in the input file.
  char units;

  /// The value of this cooling time in the units defined in 'units'.
  double coolingTime;
   
  /// This is the pointer to the next cooling time.
  CoolingTime* next;

public:
  /// Default constructor
  CoolingTime(double coolTime=COOL_HEAD, char unts=' ');

  /// Copy constructor
  CoolingTime(const CoolingTime&);

  /// Inline destructor destroys the whole list by deleting the 'next'.
  ~CoolingTime()
    { delete next; };

  /// This function reads a whole list of cooling times from the input file,
  /// attached to the stream given as an argument.
  void getCoolingTimes(istream&);

  /// This function makes an array of cooling times, all converted to
  /// seconds.
  int makeCoolingTimes(double *&);

  /// This function writes a header for the standard response table.
  void writeHeader();

  /// This function writes a header for the table of totals. 
  void writeTotalHeader(char*);

  /// This function writes an appropriately sized separator of
  /// "===...===" to frame the table.
  void writeSeparator();

  /// Inline function to return the boolean result indicating whether or
  /// not this CoolingTime is the head of the list. 
  int head() { return (coolingTime == COOL_HEAD);};
};


#endif
