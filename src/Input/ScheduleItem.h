#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list, with each object of class
Schedule having a distinct list.  A full list describes all the
items in that schedule.  The first element in each list has type
SCHED_HEAD, and contains no problem data.

 *** Class Members ***

 type : int
    Indicates what kind of item this is, based on the definitions below.

 * anonymous union *
    fluxName : char*
       The descriptive name of the flux used for a specific pulse type
       item.  This name should match one of the descriptive names in
       the Flux list.

    fluxNum : int
       During cross-referencing, the 'fluxName' descriptive name is
       converted to an ordinal number describing which flux this is.

    itemName : char*
       The descriptive name of the schedule referenced in a
       sub-schedule item.  This name should match one of the
       descriptive names in the Schedule list.

    subSched : Schedule*
       For sub-schedule items, the 'itemName' descriptive name is
       converted to a pointer to the actual schedule during
       cross-referencing.

 * anonymous union *
    pulseName : char*
       The descriptive name of the history used by this item.  This
       should match one of the descriptive names History list.

    hist : History*
       During cross-referencing, the 'pulseName' descriptive name is
       converted to a pointer to the actual History object.

 delay : double
    The time between this item ending and the next item starting,
    given in the units defined by 'dUnits'.

 opTime : double
    For pulse type items, the operation time of the pulse, given in
    units defined by 'opUnits'.
    
 dUnits : char
    The time units in which 'delay' should be interpreted.

 opUnits : char
    The time units in which 'opTime' should be interpreted.
    
 next : ScheduleItem*
    The pointer to the next ScheduleItem in the list.

 *** Member Functions ***

 * - Constructors & Destructors - *

 ScheduleItem(int, char*, char*, double, char, double, char) 
    When called with no arguments, this default constructor creates a
    blank list head with no data.  Otherwise, is creates and fills
    storage for 'itemName' (acting as 'fluxName' where necessary) and
    'pulseName', and initializes the other variables (perhaps with
    default values), with 'next' set to NULL.

 ~ScheduleItem()
    Inline destructor destroys whole list by deleting next - NOT!.

 * - Input - *

 ScheduleItem* getSubSched(char*,istream&)
    This function reads a sub-schedule type schedule item from the
    input file attached to the passed stream reference.  The
    'itemName' has already been read by the calling routine, and is
    passed as the first argument.  A pointer to the newly created
    ScheduleItem object is returned.

 ScheduleItem* getPulse(double,istream&);
    This function reads a pulse type schedule item from the input file
    attached to the passed stream reference.  The 'opTime' has already
    been read by the calling routine, and is passed as the first
    argument.  A pointer to the newly created ScheduleItem object is
    returned.

 * - xCheck - *

 void xCheck(Schedule*,Flux*,History*,char*)
    This function confirms the existence of all cross-referenced
    objects.  Once they are found, named references are usually
    converted to object pointers.  For single pulses, the flux is
    checked first, replacing the 'fluxName' with the 'fluxNum' in the
    union.  For sub-schedules, the referenced schedule is checked,
    replacing the 'itemName' with the 'subSched' pointer in the union.
    For both cases, the history is checked, replacing the 'pulseName'
    with the 'hist' pointer in the union.  The arguments are the head
    items of the lists of Schedule, Flux and History, respectively,
    followed by the name of the current schedule, for use in error
    reporting.

 void write(int)
    This function writes info about the current item to stdout using
    the argument to indicate the tabbing for hierarchical
    representation.  Single pulse items have their time, delay and
    pulsing name written, while sub-schedules recursively call back to
    the Schedule::write(int) function.

 * - Preproc - *

 void buildSched(calcSchedule*) 
    This function parses a list of ScheduleItems and adds each one to
    the calcSchedule object, pointed to by the argument, associated
    with the appropriate Schedule.  In all cases, a new calcSchedule
    object is created and passed to function
    Schedule::add(int,*calcSchedule) for inclusion in the list of
    sub-schedules.  NOTE: For each sub-schedule type item, this
    results in an extra calcSchedule object with a single
    sub-schedule.  While it is tempting to collapse this extra object
    with its sub-schedule immediately, the calcSchedule object
    corresponding to the sub-schedule may not be set yet.  This is
    done later (see calcSchedule::collapse()).

 calcSchedule* makeSchedule()
    This function initializes each calcSchedule object by counting the
    number of ScheduleItems in a list, and creating the calcSchedule
    with that number of sub-schedules.  A pointer to the newly created
    calcSchedule object is returned.

 * - Utility - *

 int head()
    Inline function to determine whether this object is the head of
    the list.  Creates boolean by comparing 'type' to SCHED_HEAD.

 */

#ifndef _SCHEDULEITEM_H
#define _SCHEDULEITEM_H

/* types of input schedule items */
#define SCHED_HEAD 0
#define SCHED_PULSE 1
#define SCHED_SCHED 2

class ScheduleItem
{
protected:
  int type;

  union
  {
    char *fluxName;
    int fluxNum;
    char *itemName;
    Schedule *subSched;
  };
  union
  {
    char    *pulseName;
    History *hist;
  };
      
  double delay,opTime;
  char dUnits, opUnits;

  ScheduleItem* next;
  
public:
  /* Service */
  ScheduleItem(int intType=SCHED_HEAD, char* name=NULL, 
	       char* pname=NULL, double inDelay=0, 
	       char inDUnits=' ', double inOpTime=0, 
	       char inOpUnits=' ');
  ~ScheduleItem(){};

  /* Input */
  ScheduleItem* getSubSched(char*,istream&);
  ScheduleItem* getPulse(double,istream&);

  /* xCheck */
  void xCheck(Schedule*,Flux*,History*,char*);
  void write(int);

  /* Preproc */
  void buildSched(calcSchedule*);
  calcSchedule* makeSchedule();

  /* Utility */
  int head() {return (type == SCHED_HEAD);};
};



#endif
