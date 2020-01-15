/* $Id: TreeInfo.C,v 1.3 2003-01-13 04:34:53 fateneja Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Chain: functions directly related to the building and analysis of chains
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "TreeInfo.h"
#include "Node.h"
#include "truncate.h"

/****************************
 ********* Service **********
 ***************************/
/** Initializes 'rank' and 'daughterNum' to 0, 'daughter' and 'parent'
    to NULL, and 'state' to CONTINUE (see truncate.h) */
TreeInfo::TreeInfo() :
  rank(0), pathNum(0), state(CONTINUE)
{
  next = NULL;
  prev = NULL;
}

/** Initializes all members with copy from old, but
    establishes new storage for daughter by invoking copy constructor
    for Node.  This should lead to a completely independent copy of
    the entire chain. */
TreeInfo::TreeInfo(const TreeInfo& t) :
  rank(t.rank), pathNum(t.pathNum), state(t.state), prev(t.prev)
{
  next = NULL;
  if (t.next != NULL)
    {
      next = new Node(*(t.next));
      memCheck(next,"TreeInfo::TreeInfo(...) copy constructor: next");
    }

}

/** Initializes 'parent', 'rank' and 'state', respectively, with
    arguments, and initializes 'daughterNum' to 0 and 'daughter' to
    NULL. */
TreeInfo::TreeInfo(Node* passedPrev, int passedRank, int passedState) :
  rank(passedRank), pathNum(0), state(passedState), prev(passedPrev)
{
  next = NULL;

}

/** Destroys the whole chain by deleteing the 'daughter'. */
TreeInfo::~TreeInfo()
{
  delete next;
  next = NULL;
}

/** The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.  Note that
    this copies the 'daughter' and 'parent' pointers (NOT the
    objects).  Since all chains appear as linear chains, it must
    therefore delete the previous sub-tree (delete 'daughter') before
    copying the new pointer. */
TreeInfo& TreeInfo::operator=(const TreeInfo& t)
{
  if (this == &t)
    return *this;

  prev = t.prev;
  rank = t.rank;
  state = t.state;
  pathNum = t.pathNum;
  delete next;
  next = NULL;

  next = t.next;

  return *this;

}
