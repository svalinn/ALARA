/* $Id: History.h,v 1.3 2000-01-17 16:57:38 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list and describes the various
pulsing histories used in the problem.  Each object is a named pulsing
history which may be referenced as part of zero, one or more
schedules.  The first element in each list is named IN_HEAD (defined
in Input.h), and contains no problem data.

 *** Class Members ***

 histName : char*
    The descriptive name is used to reference this pulsing history
    from other elements of the input.

 pulseLevelHead : PulseLevel*
    A pointer to the first PulseLevel object in the list of pulse
    levels for this pulsing history.  This pointer is used to initiate
    actions or perform actions on the whole list, rather than just a
    single PulseLevel object.

 calcHist : PulseHistory*
    This is a pointer to the PulseHistory object corresponding to this
    History object.  The input version of a pulsing history is stored
    in a History object, while the version for optimized for
    calculation is stored in a PulseHistory object.

 next : History*
    The next History in the list of possible pulsing histories.

 *** Member Functions ***

 * - Constructors & Destructors - *

 History(char *) 
    Default constructor creates a blank list head with no arguments.
    Otherwise, it sets the name of the history and initializes the
    pulse level list with a head item.  Other pointers are set to
    NULL.

 History(const History&) 
    Copy constructor is identical to the default constructor.  While
    the name is copied, the pulse level list is initialized with a new
    list head, but neither the pulsing information nor the 'calcHist'
    pointer are copied!

 ~History()
    Destructor deletes the storage for 'histName' and deletes the
    pulsing information list.  It then destroys the rest of the list
    by deleting 'next'.


 History& operator=(const History&)
    This assignment operator behaves the similarly to the copy
    constructor.  Even though new puleList and calcHist information is
    not copied from the righ hand side, the old values are deleted.
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the left hand side object will continue to
    be part of the same list unless explicitly changed.


 * - Input - *

 History *getHistory(istream&)
    Read an entire pulsing history from the input file attached to
    the passed stream reference.  After reading the descriptive name
    of the history, it read the pulsing level information into the
    pulsing level list.  Returns a pointer to the newly created
    History object.

 * - Preproc - *

 void makeHistories() 
   Called through the object at the head of the history list, this
   function onverts each object in the list to a PulseHistory object
   (for calcultion).  It acts on the entire history list, one at a
   time, and should be called through the head of the history list.
   It calls PulseLevel::makeHistory() through the head of the pulse
   level list.

 * - Utility - *

 char* getName()
    Inline function provides access to the descriptive name.  Note
    that this is not entirely robust as it simply returns the char*
    pointer, the contents of which might be (but shouldn't be)
    subsequently changed.

 History* find(char*)
    Function to search the history list for a given named history.
    If found, a pointer to that history is returned, and if not,
    NULL.

 PulseHistory* getCalcHist()
    Inline function provides access to the PulseHistory object
    corresponding to this History object, returning just the pointer.

*/

#ifndef _HISTORY_H
#define _HISTORY_H

#include "Input_def.h"

class History
{
protected:
  char *histName;
  PulseLevel *pulseLevelHead;
  PulseHistory *calcHist;

  History* next;

public:
  /* Service */
  History(char *name=IN_HEAD);
  History(const History&);
  ~History();

  History& operator=(const History&);

  /* Input */
  History *getHistory(istream&);

  /* Preproc */
  void makeHistories();

  /* Utility */
  char* getName() {return histName;};
  History* find(char*);
  PulseHistory* getCalcHist() { return calcHist; };
};


#endif
