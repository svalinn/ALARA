/* $Id: PulseHistory.h,v 1.4 2003-01-13 04:34:27 fateneja Exp $ */
#include "alara.h"

#ifndef PULSEHISTORY_H
#define PULSEHISTORY_H

#include "Matrix.h"

/** \brief This class stores the information about a pulsing history 
 *         in the problem.
 *          
 *  Each History object defined in the input will create a PulseHistory
 *  object for use in the solution phase.  These objects will be 
 *  accessed through pointers in the calcSchedule objects which define
 *  the whole schedule hierarchy.
 */

class PulseHistory
{

protected:
  /// This is used to keep track of which chain is being solved.
  /** Since a pulsing history can be referenced from many places 
      in the schedule, this prevents the code from solving the 
      decay matrices more than once per chain. (Note that decay 
      matrices are not flux dependent.) */
  int setCode; 
    
  /// The number of levels in this pulsing history.
  int nLevels;
    
  /// The number of pulses in each of the levels of the pulsing 
  /// history.
  int *nPulse;

  /// The dwell time between the pulses in each of the levels of 
  /// the pulsing history.
  double *td;

  /// An array of Matrix objects used to store the decay matrices 
  /// for this pulsing history, one per dwell time, i.e. level.
  Matrix *D;

  /// Name used to refer to each pulsing history.
  char *histName;

public:
  /// Default constructor, when called with no arguments
  PulseHistory (const char* histName, int nlvls=0, int *pulse = NULL, double *decay=NULL );

  /// Copy constructor 
  PulseHistory (const PulseHistory&);
 
  /// This constructor acts to combine two pulsing histories into one.
  PulseHistory (PulseHistory*, double, PulseHistory*);

  /// Inline destructor simply deletes all storage for objects of 
  /// class PulseHistory.
  ~PulseHistory ()
    { 
      delete [] td; 
      delete [] nPulse; 
      delete [] D; };

  /// Overloaded assignment operator
  PulseHistory& operator=(const PulseHistory&);

  /// This function checks to see if the decay matrices for the chain
  /// pointed to by the argument have already been solved.
  void setDecay(Chain*);

  /// This function performs the mathematics required to represent the
  /// pulsing history.
  Matrix doHistory(Matrix);

  /// This function writes the details of each pulse history to the
  /// output file.
  void write_ph() const;

};

#endif
