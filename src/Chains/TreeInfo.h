/* $Id: TreeInfo.h,v 1.6 2003-01-13 04:34:53 fateneja Exp $ */
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

  /// The rank of the current isotope.
  /** Note that the root isotope has rank 0. */
  int rank;

  /// A counter indicating which path of the current isotope
  /// is currently being processes.
  int pathNum;

  /// This is the storage for the current truncation state of this
  /// isotope.
  /** It will be established when Chain::setState() is
      invoked, and may change through invocations of Node::getState().
      See truncate.h for possible values of 'state'. */
  int state;
  
  /// This is the pointer to the next Node in the chain of Nodes.
  /** (A daughter in the forward direction, but a parent in the reverse
      direction mode.) */
  Node *next;

  /// This points to the previous Node in the chain of Nodes.
  /** (A parent in the forward direction, but a daughter in the reverse
      direction mode.) */
  Node *prev;

public:
  /// Default constructor 
  TreeInfo();

  /// Copy constructor
  TreeInfo(const TreeInfo&);

  /// Basic constructor 
  TreeInfo(Node*, int, int);

  /// Destructor
  ~TreeInfo();

  /// Overloaded assignment operator
  TreeInfo& operator=(const TreeInfo&);

  /// Inline function returns pointer to next isotope in chain
  Node* getNext() { return next; };
};

#endif






