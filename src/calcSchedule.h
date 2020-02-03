/* $Id: calcSchedule.h,v 1.7 2003-01-13 04:34:29 fateneja Exp $ */
#include "alara.h"

#ifndef CALCSCHEDULE_H
#define CALCSCHEDULE_H

#include "Matrix.h"

/** \brief This class is used to create a linked hierarchy of schedule
 *         information.  Each calcSchedule object represents a portion 
 *         of a potentially complex irradiaiton schedule.
 *
 *  Some notes on the creation of calcSchedule objects from the Schedule
 *  and ScheduleItem objects of the Input.
 *
 *  1. First, a calcSchedule object is created for each Schedule object
 *     in the input, by counting the number of ScheduleItems and
 *     initializing it with that size.  We will call these TYPE A
 *     calcSchedule objects.  TYPE A objects have no delay and no
 *     pulsing history.
 *
 *  2. Then, for each Schedule object, the list of ScheduleItems is
 *     parsed, and a new calcSchedule object is created for each
 *     ScheduleItem, and pointed to by the list of pointers in that
 *     Schedule's corresponding calcSchedule object.  There are two
 *     possibilities for these: a single pulse (nItems=0) or a
 *     sub-schedule (nItems=1).  We will call these TYPE B-0 and TYPE
 *     B-1, respectively.
 *
 *  3. During steps 1 and 2, the top Schedule in the hierarchy is found
 *     and its corresponding calcSchedule object is converted to a
 *     topSchedule object (derived from calcSchedule).
 *
 *  4. Finally, starting with the single topSchedule object, the
 *     hierarchy is traversed in a depth first search and the
 *     calcSchedules are "collapsed" to remove superfluous levels which
 *     have only single sub-items.  There are two (more?) cases where
 *     this is applied: TYPE A objects with single sub-items, and all
 *     TYPE B-1 objects.
 *
 *     TYPE A objects with single items point to one of the TYPE B
 *     variants: TYPE B-0 implies that a single pulse type object is
 *     being conveniently defined for reuse, while TYPE B-1 implies less
 *     than optimum user input, with a single schedule nested inside a
 *     schedule.  Since TYPE A objects have neither pulsing histories or
 *     delays, the act of collapsing these is functionally the same as
 *     replacing the TYPE A with the TYPE B object.
 *
 *     TYPE B-1 objects all point to TYPE A objects, and are combined with
 *     that TYPE A object, copying the list of sub-items from A to B,
 *     but retaining the pulsing history and delay of the TYPE B-1
 *     (since TYPE A objects have no pulsing history or delay).  We will
 *     call these TYPE B-A objects.  
 *
 *     Note that this collapsing happens from the top down, so a TYPE
 *     A(1) -> TYPE B-1 -> TYPE A(N) hierarchy will first be collapsed
 *     to a TYPE B-1 -> TYPE A(N) hierarchy, and then recursively
 *     converted to a TYPE B-A.  This also means that the lower object
 *     in the hierarchy is unchanged.  This is important since such
 *     objects could be pointed to by many calcSchedules, and they need
 *     to be collapsed independently.
 *
 *     At the end of the collapsing, the only possible TYPE A object
 *     will be a topSchedule.  Of course, if it has only one item, then
 *     it will be collapsed to a TYPE B-0 or TYPE B-A object.
 */

class calcSchedule
{
protected:
  /// Allows us to check whether or not this decay matrix has been
  /// calculated for the current chain.
  /** Since a calcSchedule object may be referenced in more than one
      place in the hierarchy, and there is storage for a decay matrix,
      this allows us to check whether or not this decay matrix has been
      calculated for the current chain.  This avoid superfluous
      calculations. */
  int setCode;
  
  /// This indicates the number of items which make up this schedule.
  int nItems;
  
  /// This is a pointer to the pulsing history which governs this
  /// schedule.
  /** Once all the items in this schedule have been applied,
      the whole result will be repeated ("pulsed") with this history. */
  PulseHistory* history;
  
  /// This is the delay following this schedule.
  /** After the all the operational items have been applied and the 
      pulsing has been performed, there may be a delay before the next
      schedule item. */
  double delay;
  
  
  /// This is the storage for the decay matrix representing the 
  /// post-schedule delay.
  /** To save time, the matrix generated to reperent the 'delay' is
      stored and only the new elements are calculated. */
  Matrix D;

  /// In the event that this schedule is a single pulse, this indicates
  /// which flux value should be used.
  int fluxCode;

  /// In the event that this schedule is a single pulse, this is the
  /// irradiation time, in seconds, of that pulse.
  double  opTime;

  /// This is an array of pointers to the calcSchedules which represent
  /// the schedule items making up this schedule.  There are 
  /// calcSchedule::nItems pointers.
  calcSchedule** subSched;

public:
  /// Default Constructor, when called with no arguments
  calcSchedule(int numItems = -1);

  /// Copy constructor 
  calcSchedule(const calcSchedule&);

  /// This constructor creates a new single pulse schedule.
  calcSchedule(double, double, History*, int);

  /// This constructor creates a new sub-schedule schedule.
  calcSchedule(double, History*, calcSchedule* );

  /// Inline destructor
  ~calcSchedule();

  /// Overloaded assignment operator
  calcSchedule& operator=(const calcSchedule&);

  /// This function is responsible for recursing through the hierarchy
  /// in a depth first search, removing superfluous layers in the
  /// hierarchy.
  void collapse();

  /// Inline function simply allows 'subSched' element indexed by the
  /// first argument to be pointed at the second argument.  
  void add(int itemNum, calcSchedule* newSched)
    { subSched[itemNum] = newSched; };

  /// This function recurses through the calcSchedule hierarchy, setting
  /// the storage matrices 'D'.
  void setDecay(Chain*);

  /// This function manages the solution of the chain passed in argument
  /// 1 on the schedule hierarchy using the storage transfer matrix
  /// hierarchy passed in second argument.
  void setT(Chain*, calcScheduleT*);

  /// In this fucntion each subschedule is solved recursively with
  /// master transfer matrix, 'calcScheduleT::opBlockT' being built by
  /// multiplication.
  void setSubTs(Chain*, calcScheduleT*);

  /// Inline function provides access to number of items in this schedule.
  int numItems() { return nItems; };

  /// Inline operator function allows C style indexing of sub-schedules.
  calcSchedule*& operator[](int idx) 
    { return subSched[idx]; };

};
    
#endif
