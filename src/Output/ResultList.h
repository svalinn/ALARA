#include "alara.h"

/* ******* Class Description ************

This class is derived publicly from class Result and is also used to
store the results of each computation.  ResultList objects serve as
both elemts of a linked list and, individually, as the head of another
linked list of Result objects.  A linked list of ResultList objects
will make up three (3) of the dimensions for each of the 4-D (interval
* input isotope * output isotope * cooling time) result arrays.  The
first element in each list has type RESULT_HEAD and contains no
problem data.  Each interval will have a linked list of ResultList
objects, one for each of the input isotopes which exist in that
interval.

 *** Class Members ***

 nextList : ResultList*
    This is a pointer to the next ResultList list, which has the
    results for a different input isotope.

 *** REINTERPRETED INHERITED MEMBERS ***

 kza : int
    This is the KZA value of the input isotope which has this list of
    results.  In the base class, this represents the KZA of the
    resultant isotope!

 *** Member Functions ***

 * - Constructors & Destructors - *

 ResultList(int)
    When called with no arguments, this inline default constructor
    creates a blank list head for a list of ResultList objects.
    Otherwise, it sets the KZA value of this ResultList object to the
    argument.  It invokes the base class constructor Result(int) and
    sets 'nextList' to NULL.

 ResultList(const ResultList&)
    This inline copy constructor invokes the base class copy
    constructor and then copies the 'nextList' pointer.  Note,
    therefore, that this does not generate a distinct ResultList list.

 ~ResultList()
    Inline destructor destroys whole list of ResultLists by deleting
    'nextList'.

 ResultList& operator=(const ResultList&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the object will continue to be part of the
    same list unless explicitly changed.

 * - Tally - *

 ResultList* tally(Chain*, topScheduleT*) 
    This funtion tallies the results of a calculation of the chain
    pointed to by the first argument by querying results from the
    topScheduleT object pointed to by the second argument.  After the
    chain is queried for the root isotope's KZA, if this is not the
    appropriate ResultList, a new one is added to the end (Note that
    since the root isotope is the outside loop of the solution phase,
    there is no need to search the list; if the last element of the
    list is not the right one, we just add one after it).  The the
    chain is queried for which rank is the first to be tallied for
    this chain and begins tallying the results for that and subsequent
    ranks into the ResultList.

 * - Postproc - *

 void postProcList(Result*,Mixture*) 
     This function steps through the list from which it was called,
     and then steps through the components of the Mixture pointed to
     by the second argument which contain each root.  For each
     component, it increments the output list, pointed to by the first
     argument, by invoking postProc() on the Result list which it
     points to and passing the output list for that component.


*/

#ifndef _RESULTLIST_H
#define _RESULTLIST_H

#include "Result.h"

#define RESULT_HEAD -1

class ResultList : public Result
{
protected:
  ResultList* nextList;

public:
  /* Service */
  ResultList(int setKza = RESULT_HEAD) :
    Result(setKza)
    { nextList = NULL; };
  ResultList(const ResultList& r) : 
    Result(r)
    { nextList = r.nextList; };
  ~ResultList()
    { delete nextList; };

  ResultList& operator=(const ResultList&);

  /* Tally */
  ResultList* tally(Chain*, topScheduleT*);

  /* Postproc */
  void postProcList(Result*,Mixture*);
};
  
#endif
