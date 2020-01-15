/* $Id: Schedule.h,v 1.5 2003-01-13 04:34:59 fateneja Exp $ */
#include "alara.h"

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "Input_def.h"

/** \brief This class is invoked as a linked list and describes the 
 *         various irradiation schedules used in the problem.  
 *  
 *  Each object is a named schedule which may be referenced as part of
 *  zero, one or more other schedules.  The first element in each list
 *  is named IN_HEAD (defined in Input.h), and contains no problem data.
 *  Each Schedule is made up of a list of items as defined in 
 *  ScheduleItem.h.
 */

class Schedule
{
protected:
  /// The descriptive name is used to reference this Schedule from 
  /// other elements of the input.
  char* schedName;

  /// A pointer to the first ScheduleItem object in the list of
  /// ScheduleItems for this Schedule.  
  /** This pointer is used to initiate actions or perform actions on 
      the whole list, rather than just a single ScheduleItem 
      object. */
  ScheduleItem *itemListHead;

  /// This is a pointer to the calcSchedule object corresponding to 
  /// this Schedule object.  
  /** While the input version of an irradiation schedule, the version
      optimized for calculation is stored in a calcSchedule object. */
  calcSchedule *calcSched;

  /// This boolean keeps track of whether or not the given Schedule is
  /// referenced as a sub-schedule.
  /** If it is not, it will later be assumed as the top schedule.  
      Therefore, only one schedule should not be referenced in this way. */
  int usedAsSub;

  // Pointer to the next schedule item in the list
  Schedule* next;

public:
  /// Default constructor
  Schedule(const char* name=IN_HEAD);

  /// Copy constructor
  Schedule(const Schedule& );

  /// Default destructor
  ~Schedule();

  /// Overloaded assignment operator
  Schedule& operator=(const Schedule&);
  
  /// Read an entire schedule from the input file attached to the passed
  /// stream reference.
  Schedule* getSchedule(istream&);
  
  /// Inline interface function sets the 'usedAsSub' flag to TRUE;
  void use() {usedAsSub = TRUE;};

  /// Cross-check that the fluxes and histories referenced in the
  /// schedule items exists.
  void xCheck(Flux*,History*);

  /// This function ouputs the hierarchical schedule structure to STDOUT.
  void write(int level=0, char *histName = NULL, double delay=0, char dUnits=' ');

  /// This function loops through all the schedules and creates an
  /// object of class calcSchedule for each one by calling
  /// ScheduleItem::makeSchedule().  
  topSchedule* makeSchedules(CoolingTime*);
  
  /// Find a specific schedule in the list based on the argument which
  /// is compared to 'schedName'.  
  Schedule* find(char*);

  /// Inline access to the pointer to the calcSchedule object corresponding 
  /// to this Schedule object.
  calcSchedule* getCalcSched() { return calcSched; };

  /// Inline access to the name of this schedule.
  char* getName() { return schedName;};
};


#endif
