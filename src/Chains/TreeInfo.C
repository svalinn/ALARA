/* $Id: TreeInfo.C,v 1.2 1999-08-24 22:06:18 wilson Exp $ */
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
/* basic constructor for TreeInfo base class */
TreeInfo::TreeInfo() :
  rank(0), pathNum(0), state(CONTINUE)
{
  next = NULL;
  prev = NULL;
}

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

TreeInfo::TreeInfo(Node* passedPrev, int passedRank, int passedState) :
  rank(passedRank), pathNum(0), state(passedState), prev(passedPrev)
{
  next = NULL;

}

/* basic destructor for TreeInfo */
TreeInfo::~TreeInfo()
{
  delete next;
  next = NULL;
}

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

