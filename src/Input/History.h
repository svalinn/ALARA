/* $Id: History.h,v 1.5 2003-01-13 04:34:56 fateneja Exp $ */
#include "alara.h"

#ifndef _HISTORY_H
#define _HISTORY_H

#include "Input_def.h"

/** \brief This class is invoked as a linked list and describes the 
 *         various pulsing histories used in the problem.
 *
 *  Each object is a named pulsing history which may be referenced
 *  as part of zero, one or more schedules.  The first element in
 *  each list is named IN_HEAD (defined in Input.h), and contains no
 *  problem data.
 */

class History
{
protected:
  /// The descriptive name is used to reference this pulsing history
  /// from other elements of the input.
  char *histName;

  /// A pointer to the first PulseLevel object in the list of pulse
  /// levels for this pulsing history.  
  /** This pointer is used to initiate actions or perform actions 
      on the whole list, rather than just a single PulseLevel 
      object. */
  PulseLevel *pulseLevelHead;

  /// This is a pointer to the PulseHistory object corresponding to 
  /// this History object.
  /** The input version of a pulsing history is stored in a History
      object, while the version for optimized for calculation is 
      stored in a PulseHistory object. */
  PulseHistory *calcHist;

  /// The next History in the list of possible pulsing histories.
  History* next;

public:
  /// Default constructor
  History(char *name=IN_HEAD);

  /// Copy constructor
  History(const History&);

  /// Default destructor
  ~History();

  /// Overloaded assignment operator
  History& operator=(const History&);

  /// Read an entire pulsing history from the input file attached to
  /// the passed stream reference.
  History *getHistory(istream&);

  /// This function onverts each object in the list to a PulseHistory 
  /// object
  void makeHistories();

  /// Inline function provides access to the descriptive name.
  char* getName() {return histName;};

  /// Function to search the history list for a given named history.
  History* find(char*);

  /// Inline function provides access to the PulseHistory object
  /// corresponding to this History object
  PulseHistory* getCalcHist() { return calcHist; };
};


#endif
