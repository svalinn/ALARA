#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list and describes the various
irradiation schedules used in the problem.  Each object is a named
schedule which may be referenced as part of zero, one or more other
schedules.  The first element in each list is named IN_HEAD (defined
in Input.h), and contains no problem data.  Each Schedule is made up
of a list of items as defined in ScheduleItem.h.

 *** Class Members ***

 schedName : char*
    The descriptive name is used to reference this Schedule from other
    elements of the input.

 itemListHead : ScheduleItem*
    A pointer to the first ScheduleItem object in the list of
    ScheduleItems for this Schedule.  This pointer is used to initiate
    actions or perform actions on the whole list, rather than just a
    single ScheduleItem object.

 calcSched : calcSchedule*
    This is a pointer to the calcSchedule object corresponding to this
    Schedule object.  While the input version of an irradiation
    schedule, the version optimized for calculation is stored in a
    calcSchedule object.

 usedAsSub : int
    This boolean keeps track of whether or not the given Schedule is
    referenced as a sub-schedule.  If it is not, it will later be
    assumed as the top schedule.  Therefore, only one schedule should
    not be referenced in this way.

 *** Member Functions ***

 * - Constructors & Destructors - *

 Schedule(char*)
    When called with no arguments, the default constructor creates a
    blank list head.  Otherwise, it creates and fills the storage for
    'schedName', initializes 'usedAsSub' to FALSE, creates a blank
    list of ScheduleItems, and initializes 'next' to NULL.

 Schedule(const Schedule& s)
    Copy constructor is identical to default constructor.  Therefore,
    the name is copied, but the itemList and subsequent list item
    'next' are not.  The 'usedAsSub' variable is specially copied.

 ~Schedule()
    Destructor deletes space for 'schedName', deletes itemList
    and then destroys list of Schedules by deleting 'next'.

 Schedule& operator=(const Schedule&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the object will continue to be part of the
    same list unless explicitly changed.

 * - Input - *

 Schedule* getSchedule(istream&)
    Read an entire schedule from the input file attached to the passed
    stream reference.  A list of schedule items is read until the
    keyword 'end'.
 
 * - xCheck - *

 void use()
    Inline interface function sets the 'usedAsSub' flag to TRUE;

 void xCheck(Flux*,History*)
    Cross-check that the fluxes and histories referenced in the
    schedule items exists.  This function sets up the cross-check by
    looping through the schedules, and passing the 2 arguments (the
    head of the Flux list and the head of the History list) on to the
    list of ScheduleItems.

 void write(int, char*, double, char)
    To ensure that the parsing of the schedules was done correctly,
    the hierarchical schedule structure will be output on STDOUT.  The
    arguments indicate which level this schedule is at in the
    hierarchy, which history it is pulsed with and the delay after the
    item.  For each ScheduleItem, a similar routine is called, which
    may recursively call back to this routine.

 * - Preproc - *

 topSchedule* makeSchedules(CoolingTime*)
    This function loops through all the schedules and creates an
    object of class calcSchedule for each one by calling
    ScheduleItem::makeSchedule().  Simultaneously, it finds the
    Schedule object which is at the top of the hierarchy and converts
    it to a topSchedule object (derived from calcSchedule) and sets
    its after-shutdown cooling times from the first argument.  The
    pointer to this topSchedule object is returned.
 
 * - Utility - *

 Schedule* find(char*)
    Find a specific schedule in the list based on the argument which
    is compared to 'schedName'.  If found, returns the pointer to the
    appropriate Schedule object, otherwise, NULL.

 calcSchedule* getCalcSched()
    Inline access to the pointer to the calcSchedule object
    corresponding to this Schedule object.

 */

#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#include "Input_def.h"

class Schedule
{
protected:
  char* schedName;
  ScheduleItem *itemListHead;
  calcSchedule *calcSched;
  int usedAsSub;

  Schedule* next;

public:
  /* Service */
  Schedule(char* name=IN_HEAD);
  Schedule(const Schedule& );
  ~Schedule();

  Schedule& operator=(const Schedule&);
  
  /* Input */
  Schedule* getSchedule(istream&);
  
  /* xCheck */
  void use() {usedAsSub = TRUE;};
  void xCheck(Flux*,History*);
  void write(int level=0, char *histName = NULL, double delay=0, char dUnits=' ');

  /* Preproc */
  topSchedule* makeSchedules(CoolingTime*);
  
  /* Utility */
  Schedule* find(char*);
  calcSchedule* getCalcSched() { return calcSched; };
  char* getName() { return schedName;};
};


#endif
