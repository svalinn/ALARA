/* $Id: PulseLevel.h,v 1.4 2002-08-05 20:23:17 fateneja Exp $ */
#include "alara.h"

#ifndef _PULSELEVEL_H
#define _PULSELEVEL_H

/* pulsing head */
#define PULSE_HEAD -1

/** \brief This class is invoked as a linked list, with each object of 
 *         class History having a distinct list.  
 *
 *  A full list describes all the pulsing levels of that history.  The 
 *  first element in each list has type PULSE_HEAD (stored in the 
 *  numPulses variable), and contains no problem data. 
 */

class PulseLevel
{
protected:
  int
    /// The number of pulses at this level of pulsing.
    nPulses;

  char 
    /// The units for the delay time between pulses.
    units;

  double 
    /// Time between pulses at this level, in the units defined by 'units'.
    delay;

  PulseLevel* 
    /// The pointer to the next PulseLevel object.
    next;

public:
  /// Default constructor
  /** When called with no arguments, default inline constructor creates
      a blank list head.  Otherwise, it sets the number of pulses, the
      time and the units, and initializes the 'next' pointer to NULL. */
  PulseLevel(int inNumPulse=PULSE_HEAD, double inDelay=0, 
	     char inUnits=' ') 
    : nPulses(inNumPulse),  units(inUnits),  
      delay(inDelay), next(NULL) {};

  /// Copy constructor
  /** Inline copy constructor is identical to default constructor.
      Therefore, the number of pulses, time and units are copied, but
      not the subsequent list item 'next'. */
  PulseLevel(const PulseLevel &p)
    : nPulses(p.nPulses),  units(p.units),  
      delay(p.delay), next(NULL) {};

  /// Inline destructor destroys the entire list by deleting 'next'.
  ~PulseLevel()
    { delete next; };

  /// Overload assignment operator 
  /** This assignmnent operator behaves similarly to the copy
      constructor.  The correct implementation of this operator must
      ensure that previously allocated space is returned to the free
      store before allocating new space into which to copy the
      object. Note that 'next' is NOT copied, the object will continue to
      be part of the same list unless explicitly changed. */
  PulseLevel& operator=(const PulseLevel&);

  /// Reads a single set of pulse level information from the input file
  /// attached to passed stream reference. 
  /** It returning a pointer to the newly created PulseLevel object. The 
      first argument is the number of pulses, which must be read by the 
      calling function before calling this function. */
  PulseLevel* getPulseLevel(int, istream&);

  /// It loops through all the levels, converting the time to seconds and
  /// accumulating the data in standard arrays to create a new PulseHistory
  /// object.
  /** This function should be called through the head of the PulseLevel
      list within a particular History object. A pointer to the new object
      is returned. */
  PulseHistory* makeHistory();

  /// Inline function to determine whether this object is the head of
  /// the list.
  /** Creates boolean by comparing 'nPulses' to PULSE_HEAD. */
  int head() {return (nPulses == PULSE_HEAD);};

};

#endif
