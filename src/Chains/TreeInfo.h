/* $Id: TreeInfo.h,v 1.5 2002-08-05 20:23:14 fateneja Exp $ */
#include "alara.h"

#ifndef _TREEINFO_H
#define _TREEINFO_H

/** \brief This class stores the tree information for a particular 
 *         isotope in the chain.
 *
 *  It is not intended that there be an instance of class TreeInfo,
 *  but just that is serves as a base class for class Node.
 */

class TreeInfo
{
protected:

  int
	/// The rank of the current isotope.
	/** Note that the root isotope has rank 0. */
	rank,

	/// A counter indicating which path of the current isotope
	/// is currently being processes.
	pathNum,

	/// This is the storage for the current truncation state of this
    /// isotope.
	/** It will be established when Chain::setState() is
        invoked, and may change through invocations of Node::getState().
        See truncate.h for possible values of 'state'. */
	state;

  Node 
	/// This is the pointer to the next Node in the chain of Nodes.
	/** (A daughter in the forward direction, but a parent in the reverse
        direction mode.) */
    *next,

	/// This points to the previous Node in the chain of Nodes.
	/** (A parent in the forward direction, but a daughter in the reverse
	    direction mode.) */
	*prev;

public:
  /// Default constructor 
  /** Initializes 'rank' and 'daughterNum' to 0, 'daughter' and 'parent' 
      to NULL, and 'state' to CONTINUE (see truncate.h) */
  TreeInfo();

  /// Copy constructor
  /** Initializes all members with copy from old, but
      establishes new storage for daughter by invoking copy constructor
      for Node.  This should lead to a completely independent copy of
      the entire chain. */
  TreeInfo(const TreeInfo&);

  /// Basic constructor 
  /** Initializes 'parent', 'rank' and 'state', respectively, with 
      arguments, and initializes 'daughterNum' to 0 and 'daughter' to 
      NULL. */
  TreeInfo(Node*, int, int);

  /// Destructor
  /** Destroys the whole chain by deleteing the 'daughter'. */
  ~TreeInfo();

  /// Overloaded assignment operator
  /** The correct implementation of this operator must ensure that
      previously allocated space is returned to the free store before
      allocating new space into which to copy the object.  Note that
      this copies the 'daughter' and 'parent' pointers (NOT the
      objects).  Since all chains appear as linear chains, it must
      therefore delete the previous sub-tree (delete 'daughter') before
      copying the new pointer. */
  TreeInfo& operator=(const TreeInfo&);

  /// Inline function returns pointer to next isotope in chain
  Node* getNext() { return next; };
};

#endif






