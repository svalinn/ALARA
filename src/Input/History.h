/* $Id: History.h,v 1.4 2002-08-05 20:23:16 fateneja Exp $ */
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
  char 
    /// The descriptive name is used to reference this pulsing history
    /// from other elements of the input.
    *histName;

  PulseLevel 
    /// A pointer to the first PulseLevel object in the list of pulse
    /// levels for this pulsing history.  
    /** This pointer is used to initiate actions or perform actions 
        on the whole list, rather than just a single PulseLevel 
        object. */
    *pulseLevelHead;

  PulseHistory 
    /// This is a pointer to the PulseHistory object corresponding to 
    /// this History object.
    /** The input version of a pulsing history is stored in a History
        object, while the version for optimized for calculation is 
        stored in a PulseHistory object. */
    *calcHist;

  History* 
    /// The next History in the list of possible pulsing histories.
    next;

public:
  /// Default constructor
  /** This constructor creates a blank list head with no arguments.
      Otherwise, it sets the name of the history and initializes the
      pulse level list with a head item.  Other pointers are set to
      NULL. */
  History(char *name=IN_HEAD);

  /// Copy constructor
  /** This constructor is identical to the default constructor.  While
      the name is copied, the pulse level list is initialized with a new
      list head, but neither the pulsing information nor the 'calcHist'
      pointer are copied! */
  History(const History&);

  /// Default destructor
  /** Destructor deletes the storage for 'histName' and deletes the
      pulsing information list.  It then destroys the rest of the list
      by deleting 'next'. */
  ~History();

  /// Overloaded assignment operator
  /** This assignment operator behaves the similarly to the copy
      constructor.  Even though new puleList and calcHist information is
      not copied from the righ hand side, the old values are deleted.
      The correct implementation of this operator must ensure that
      previously allocated space is returned to the free store before
      allocating new space into which to copy the object. Note that
      'next' is NOT copied, the left hand side object will continue to
      be part of the same list unless explicitly changed. */
  History& operator=(const History&);

  /// Read an entire pulsing history from the input file attached to
  /// the passed stream reference.
  /** After reading the descriptive name of the history, it read the
      pulsing level information into the pulsing level list.  Returns a
      pointer to the newly created History object. */
  History *getHistory(istream&);

  /// This function onverts each object in the list to a PulseHistory 
  /// object
  /** This function is called through the object at the head of the history
      list. It acts on the entire history list, one at a time, and should 
      be called through the head of the history list. It calls 
      PulseLevel::makeHistory() through the head of the pulse level 
      list. */
  void makeHistories();

  /// Inline function provides access to the descriptive name.
  /** Note that this is not entirely robust as it simply returns the 
      char* pointer, the contents of which might be (but shouldn't be)
      subsequently changed. */
  char* getName() {return histName;};

  /// Function to search the history list for a given named history.
  /** If found, a pointer to that history is returned, and if not,
      NULL. */
  History* find(char*);

  /// Inline function provides access to the PulseHistory object
  /// corresponding to this History object, returning just the 
  /// pointer.
  PulseHistory* getCalcHist() { return calcHist; };
};


#endif
