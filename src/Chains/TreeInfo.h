/* $Id: TreeInfo.h,v 1.4 2000-02-11 20:55:19 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class stores the tree information for a particular isotope in the
chain.  It is not intended that there be an instance of class
TreeInfo, but just that is serves as a base class for class Node.

 *** Class Members ***

 rank : int
    The rank of the current isotope.  Note that the root isotope has
    rank 0.

 pathNum : int
    A counter indicating which path of the current isotope is
    currently being processes.

 state : int
    This is the storage for the current truncation state of this
    isotope.  It will be established when Chain::setState() is
    invoked, and may change through invocations of Node::getState().
    See truncate.h for possible values of 'state'.

 next : Node*

    This is the pointer to the next Node in the chain of Nodes.  (A
    daughter in the forward direction, but a parent in the reverse
    direction mode.)

 prev : Node* 
    This points to the previous Node in the chain of Nodes.  (A
    parent in the forward direction, but a daughter in the reverse
    direction mode.)

 *** Member Functions ***

 * - Constructors and Destructors - *

 TreeInfo()
     Default constructor initializes 'rank' and 'daughterNum' to 0,
    'daughter' and 'parent' to NULL, and 'state' to CONTINUE (see
    truncate.h)

 TreeInfo(const TreeInfo&)
    Copy constructor initializes all members with copy from old, but
    establishes new storage for daughter by invoking copy constructor
    for Node.  This should lead to a completely independent copy of
    the entire chain.

 TreeInfo(Node*, int, int)
    Basic constructor initializes 'parent', 'rank' and 'state',
    respectively, with arguments, and initializes 'daughterNum' to 0
    and 'daughter' to NULL.

 ~TreeInfo()
    Destructor destroys the whole chain by deleting 'daughter'.

 TreeInfo& operator=(const TreeInfo&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.  Note that
    this copies the 'daughter' and 'parent' pointers (NOT the
    objects).  Since all chains appear as linear chains, it must
    therefore delete the previous sub-tree (delete 'daughter') before
    copying the new pointer.

 * - Access - *

 Node* getNext()
    Inline function returns pointer to next isotope in chain.

 */

#ifndef _TREEINFO_H
#define _TREEINFO_H

class TreeInfo
{
protected:
  int rank, pathNum, state;
  Node *next, *prev;

public:
  /* Service */
  TreeInfo();
  TreeInfo(const TreeInfo&);
  TreeInfo(Node*, int, int);
  ~TreeInfo();

  TreeInfo& operator=(const TreeInfo&);
  Node* getNext() { return next; };

};

#endif






