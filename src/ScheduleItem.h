/* $Id: ScheduleItem.h,v 1.5 2003-01-13 04:34:59 fateneja Exp $ */
#include "alara.h"

#ifndef SCHEDULEITEM_H
#define SCHEDULEITEM_H

/* types of input schedule items */
#define SCHED_HEAD 0
#define SCHED_PULSE 1
#define SCHED_SCHED 2

/** \brief This class is invoked as a linked list, with each object of 
 *         class Schedule having a distinct list.  
 *
 *  A full list describes all the items in that schedule.  The first 
 *  element in each list has type SCHED_HEAD, and contains no problem 
 *  data.
 */

class ScheduleItem
{
protected:
  /// Indicates what kind of item this is, based on the definitions below.
  int type;

  union
  {
    /// The descriptive name of the flux used for a specific pulse type
    /// item.  
    /** This name should match one of the descriptive names in the Flux
	list. */
    char *fluxName;

    /// During cross-referencing, the 'fluxName' descriptive name is
    /// converted to an ordinal number describing which flux this is.
    int fluxNum;

    /// The descriptive name of the schedule referenced in a
    /// sub-schedule item.
    /** This name should match one of the descriptive names in the 
	Schedule list. */
    char *itemName;

    /// For sub-schedule items, the 'itemName' descriptive name is
    /// converted to a pointer to the actual schedule during
    /// cross-referencing.
    Schedule *subSched;
  };

  union
  {
    /// The descriptive name of the history used by this item.
    /** This should match one of the descriptive names History list. */
    char *pulseName;

    /// During cross-referencing, the 'pulseName' descriptive name is
    /// converted to a pointer to the actual History object.
    History *hist;
  };
      
  /// The time between this item ending and the next item starting,
  /// given in the units defined by 'dUnits'.
  double delay;
    
  /// For pulse type items, the operation time of the pulse, given in
  /// units defined by 'opUnits'.
  double opTime;

  /// The time units in which 'delay' should be interpreted.
  char dUnits;
    
  /// The time units in which 'opTime' should be interpreted.
  char opUnits;

  /// The pointer to the next ScheduleItem in the list.
  ScheduleItem* next;
  
public:
  /// Default constructor
  ScheduleItem(int intType=SCHED_HEAD, char* name=NULL, 
	       char* pname=NULL, double inDelay=0, 
	       char inDUnits=' ', double inOpTime=0, 
	       char inOpUnits=' ');

  /// Inline destructor destroys whole list by deleting next - NOT!.
  ~ScheduleItem()
    { delete next; };

  /// This function reads a sub-schedule type schedule item from the
  /// input file attached to the passed stream reference.
  ScheduleItem* getSubSched(char*,istream&);

  /// This function reads a pulse type schedule item from the input file
  /// attached to the passed stream reference.  
  ScheduleItem* getPulse(double,istream&);

  /// This function confirms the existence of all cross-referenced
  /// objects.  
  void xCheck(Schedule*,Flux*,History*,char*);

  /// This function writes info about the current item to stdout 
  void write(int);

  /// This function parses a list of ScheduleItems and adds each one to
  /// the calcSchedule object, pointed to by the argument, associated
  /// with the appropriate Schedule.
  void buildSched(calcSchedule*);

  /// This function initializes each calcSchedule object
  calcSchedule* makeSchedule();

  /// Inline function to determine whether this object is the head of
  /// the list.  Creates boolean by comparing 'type' to SCHED_HEAD.
  int head() {return (type == SCHED_HEAD);};
};



#endif
