/* $Id: CoolingTime.h,v 1.3 2000-01-17 16:57:38 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is a linked list of the input information for after-shutdown
cooling times.  The single object of class Input has a list of cooling
times.  The first element in each list has type COOL_HEAD (defined through
the 'coolingTime' member), and contains no problem data.

 *** Class Members ***

 units : char
    This indicates the time units specified in the input file.

 coolingTime : double
    The value of this cooling time in the units defined in 'units'.

 next : CoolingTime*
    This is the pointer to the next cooling time.

 *** Member Functions ***

 * - Constructors & Destructors - *

 CoolingTime(double , char )
    Default constructor creates a blank list head with no arguments.
    Otherwise, it sets both the time and units and initializes the
    'next' pointer to NULL.

 CoolingTime(const CoolingTime&)
    Copy constructor copies scalar members and sets 'next' to NULL.

 ~CoolingTime()
    Inline destructor destroys the whole list by deleting the 'next'.

 * - Input - *

 void getCoolingTimes(istream&)
    This function reads a whole list of cooling times from the input file,
    attached to the stream given as an argument.  It reads a list of times
    and units until it finds the keyword "end".  It is called through the
    head of the cooling times list.

 * - Preproc - *

 int makeCoolingTimes(double *&)
    This function makes an array of cooling times, all converted to
    seconds.  Called through the head of the cooling times list, it
    allocates the correct number of doubles, assigning them to the pointer
    reference argument.  It returns the number of cooling times.

 * - Postproc - *

 void writeHeader()
    This function writes a header for the standard response table,
    with a colume for the isotope, a column for the @shutdown result,
    and then a column for each after-shutdown cooling time.

 void writeTotalHeader(char*)
    This function writes a header for the table of totals, with one
    column indicating the counter for the total in question, one
    column for @ shutdown results, and then one column for each of the
    after-shutdown cooling times.

 void writeSeparator()
    This function writes an appropriately sized separator of
    "===...===" to frame the table.

 * - Utility - *

 int head()
    Inline function to return the boolean result indicating whether or
    not this CoolingTime is the head of the list. 

 */

#ifndef _COOLINGTIME_H
#define _COOLINGTIME_H

/* cooling time head */
#define COOL_HEAD -1

class CoolingTime
{
protected:
  char units;
  double coolingTime;
  
  CoolingTime* next;

public:
  /* service */
  CoolingTime(double coolTime=COOL_HEAD, char unts=' ');
  CoolingTime(const CoolingTime&);
  ~CoolingTime()
    { delete next; };

  /* Input */
  void getCoolingTimes(istream&);

  /* Preproc */
  int makeCoolingTimes(double *&);

  /* Postproc */
  void writeHeader();
  void writeTotalHeader(char*);
  void writeSeparator();

  /* Utility */
  int head() { return (coolingTime == COOL_HEAD);};
};


#endif
