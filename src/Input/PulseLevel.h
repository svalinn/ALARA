/* $Id: PulseLevel.h,v 1.2 1999-08-24 22:06:22 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list, with each object of class
History having a distinct list.  A full list describes all the pulsing
levels of that history.  The first element in each list has type
PULSE_HEAD (stored in the numPulses variable), and contains no problem
data.

 *** Class Members ***

 nPulses : int
    The number of pulses at this level of pulsing.

 units : char
    The units for the delay time between pulses.

 delay : double
    Time between pulses at this level, in the units defined by 'units'.

 next : PulseLevel*
    The pointer to the next PulseLevel object.

 *** Member Functions ***

 * - Constructors & Destructors - *

 PulseLevel(int, double, char)
    When called with no arguments, default constructor creates a blank
    list head.  Otherwise, it sets the number of pulses, the time and
    the units, and initializes the 'next' pointer to NULL.

 PulseLevel(const PulseLevel&)
    Copy constructor is identical to default constructor.  Therefore,
    the number of pulses, time and units are copied, but not the
    subsequent list item 'next'.

 ~PulseLevel()
    Inline destructor destroys the entire list by deleting 'next'.

 PulseLevel& operator=(const PulseLevel&);
   The correct implementation of this operator must ensure that
   previously allocated space is returned to the free store before
   allocating new space into which to copy the object. Note that
   'next' is NOT copied, the object will continue to be part of the
   same list unless explicitly changed.


 * - Input - *

 PulseLevel* getPulseLevel(int, istream&)
    Reads a single set of pulse level information from the input file
    attached to passed stream reference, returning a pointer to the
    newly created PulseLevel object.  The first argument is the number
    of pulses, which must be read by the calling function before
    calling this function.

 * - Preproc - *

 PulseHistory* makeHistory()
    This function should be called through the head of the PulseLevel
    list within a particular History object.  It loops through all the
    levels, converting the time to seconds and accumulating the data
    in standard arrays to create a new PulseHistory object.  A pointer
    to this new object is returned.

 * - Utility - *

 int head() 
    Inline function to determine whether this object is the head of
    the list.  Creates boolean by comparing 'nPulses' to PULSE_HEAD.
 
 */

#ifndef _PULSELEVEL_H
#define _PULSELEVEL_H

/* pulsing head */
#define PULSE_HEAD -1

class PulseLevel
{
protected:
  int nPulses;
  char units;
  double delay;

  PulseLevel* next;

public:
  /* Service */
  PulseLevel(int inNumPulse=PULSE_HEAD, double inDelay=0, char inUnits=' ');
  PulseLevel(const PulseLevel&);
  ~PulseLevel()
    { delete next; };

  PulseLevel& operator=(const PulseLevel&);

  /* Input */
  PulseLevel* getPulseLevel(int, istream&);

  /* Preproc */
  PulseHistory* makeHistory();

  /* Utility */
  int head() {return (nPulses == PULSE_HEAD);};

};


#endif
