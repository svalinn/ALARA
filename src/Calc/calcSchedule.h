#include "alara.h"

/* ******* Class Description ************

This class is used to create a linked hierarchy of schedule
information.  Each calcSchedule object represents a portion of the
schedule system.  

* Some notes on the creation of calcSchedule objects from the Schedule
  and ScheduleItem objects of the Input.

  1. First, a calcSchedule object is created for each Schedule object
     in the input, by counting the number of ScheduleItems and
     initializing it with that size.  We will call these TYPE A
     calcSchedule objects.  TYPE A objects have no delay and no
     pulsing history.

  2. Then, for each Schedule object, the list of ScheduleItems is
     parsed, and a new calcSchedule object is created for each
     ScheduleItem, and pointed to by the list of pointers in that
     Schedule's corresponding calcSchedule object.  There are two
     possibilities for these: a single pulse (nItems=0) or a
     sub-schedule (nItems=1).  We will call these TYPE B-0 and TYPE
     B-1, respectively.

  3. During steps 1 and 2, the top Schedule in the hierarchy is found
     and its corresponding calcSchedule object is converted to a
     topSchedule object (derived from calcSchedule).

  4. Finally, starting with the single topSchedule object, the
     hierarchy is traversed in a depth first search and the
     calcSchedules are "collapsed" to remove superfluous levels which
     have only single sub-items.  There are two (more?) cases where
     this is applied: TYPE A objects with single sub-items, and all
     TYPE B-1 objects.

     TYPE A objects with single items point to one of the TYPE B
     variants: TYPE B-0 implies that a single pulse type object is
     being conveniently defined for reuse, while TYPE B-1 implies less
     than optimum user input, with a single schedule nested inside a
     schedule.  Since TYPE A objects have neither pulsing histories or
     delays, the act of collapsing these is functionally the same as
     replacing the TYPE A with the TYPE B object.

     TYPE B-1 objects all point to TYPE A objects, and are combined with
     that TYPE A object, copying the list of sub-items from A to B,
     but retaining the pulsing history and delay of the TYPE B-1
     (since TYPE A objects have no pulsing history or delay).  We will
     call these TYPE B-A objects.  

     Note that this collapsing happens from the top down, so a TYPE
     A(1) -> TYPE B-1 -> TYPE A(N) hierarchy will first be collapsed
     to a TYPE B-1 -> TYPE A(N) hierarchy, and then recursively
     converted to a TYPE B-A.  This also means that the lower object
     in the hierarchy is unchanged.  This is important since such
     objects could be pointed to by many calcSchedules, and they need
     to be collapsed independently.

     At the end of the collapsing, the only possible TYPE A object
     will be a topSchedule.  Of course, if it has only one item, then
     it will be collapsed to a TYPE B-0 or TYPE B-A object.

 *** Class Members ***

 setCode : int 
    Since a calcSchedule object may be referenced in more than one
    place in the hierarchy, and there is storage for a decay matrix,
    this allows us to check whether or not this decay matrix has been
    calculated for the current chain.  This avoid superfluous
    calculations.

 nItems : int
    This indicates the number of items which make up this schedule.

 history : PulseHistory*
    This is a pointer to the pulsing history which governs this
    schedule.  Once all the items in this schedule have been applied,
    the whole result will be pulsed with this schedule.

 delay : double
    This is the delay following this schedule.  After the all the
    operational items have been applied and the pulsing has been
    performed, there may be a delay before the next schedule item.

 D : Matrix 
    This is the storage for the decay matrix.  To save time, the
    matrix generated to reperent the 'delay' is stored and only the
    new elements are calculated.

 fluxCode : int
    In the event that this schedule is a single pulse, this indicates
    which flux value should be used.

 opTime : double
    In the event that this schedule is a single pulse, this is the
    irradiation time, in seconds, of that pulse.

 subSched : calcSchedule** 
    This is an array of pointers to the calcSchedules which represent
    the schedule items making up this schedule.  There are 'nItems'
    pointers.

 *** Member Functions ***

 * - Constructors & Destructors - *

 calcSchedule(int)
    When called with no arguments, this constructor makes an empty
    object with various members set to 0 or NULL except: D is set to
    an Identity matrix of size 0, and fluxCode is initialized to -1.
    Otherwise, 'nItems' is initialized with the argument and storage
    is allocated for the 'nItems' pointers in 'subSched'. (TYPE A in
    description above.)

 calcSchedule(const calcSchedule&)
    This copy constructor copies everything on a member-by-member
    basis except 'subSched' which is copied on an element-by-element
    basis.  Note that each pointer of 'subSched' is copied, and not
    the objects which each of the pointers points to.

 calcSchedule(double, double, History*, int)
    This constructor creates a new single pulse schedule.  The primary
    characteristics of a single pulse calcSchedule object are that the
    'nItems' is set to 0, and 'fluxCode' and 'opTime' are set.  (TYPE
    B-0 in description above.)

 calcSchedule(double, History*, calcSchedule* )
    This constructor creates a new sub-schedule schedule.  The primary
    characteristics of this kind of calcSchedule object are that
    'nItems' is set to 1, 'subSched' has dimension 1, and its
    first/only element points to the subsequent calcSchedule
    object. (TYPE B-1 in description above.)

 ~calcSchedule()
    Inline destructor deletes the storage for 'subSched' pointers but
    not the objects that they point to!  This does not destroy the
    entire hierarchy.

 calcSchedule& operator=(const calcSchedule&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.  Note that
    each pointer of 'subSched' is copied, and not the objects which
    each of the pointers points to.

 * - Preproc - *

 void collapse()
    This function is responsible for recursing through the hierarchy
    in a depth first search, removing superfluous layers in the
    hierarchy.  See point (4.) above for more details on the
    collapsing algorithm.

 void add(int, calcSchedule*)
    Inline function simply allows 'subSched' element indexed by the
    first argument to be pointed at the second argument.

 * - Solution - *

 void setDecay(Chain*)
    This function recurses through the calcSchedule hierarchy, setting
    the storage matrices 'D'.  After confirming that this schedule has
    not already been processed, the decay matrix 'D' for this schedule
    is set, PulseHistory::setDecay(...) is called on this
    calcSchedule's 'history' member, and the recursion takes place.

 void setT(Chain*, calcScheduleT*) 
    This function manages the solution of the chain passed in argument
    1 on the schedule hierarchy using the storage transfer matrix
    hierarchy passed in second argument.  If compound schedule,
    setSubTs(...) is called to recurse and build master transfer
    matrix. If single pulse (end state of recursion),
    'Chain::fillTMat(...)' is called to fill the 'opBlockT' transfer
    matrix.  Following this, the pulsing history is applied to fill
    'calcScheduleT::histT' and then the final delay on the schedule is
    applied to fill 'calcScheduleT::totalT'.

 void setSubTs(Chain*, calcScheduleT*)
    In this fucntion each subschedule is solved recursively with
    master transfer matrix, 'calcScheduleT::opBlockT' being built by
    multiplication.  This small code segment is separated out of
    setT(...) to allow it to be called through a topSchedule derived
    class object and still access the appropriate members.
 
 */

#ifndef _CALCSCHEDULE_H
#define _CALCSCHEDULE_H

#include "Util/Matrix.h"

class calcSchedule
{
protected:
  int setCode;

  /* all schedules */
  int nItems;

  PulseHistory *history;
  double delay;
  Matrix D;

  /* single pulses */
  int fluxCode;
  double opTime;

  /* multiple sub-schedules */
  calcSchedule **subSched;


public:
  /* Service */
  calcSchedule(int numItems = -1);
  calcSchedule(const calcSchedule&);
  calcSchedule(double, double, History*, int);
  calcSchedule(double, History*, calcSchedule* );
  ~calcSchedule();


  calcSchedule& operator=(const calcSchedule&);

  /* Preproc */
  void collapse();
  void add(int itemNum, calcSchedule* newSched)
    { subSched[itemNum] = newSched; };

  /* Solution */
  void setDecay(Chain*);
  void setT(Chain*, calcScheduleT*);
  void setSubTs(Chain*, calcScheduleT*);

  /* Utility */
  int numItems() { return nItems; };
  calcSchedule*& operator[](int idx) 
    { return subSched[idx]; };

};
    
#endif
