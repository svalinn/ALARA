/* $Id: Root.h,v 1.9 2002-08-05 20:23:14 fateneja Exp $ */
#include "alara.h"

#ifndef _ROOT_H
#define _ROOT_H

#include "Node.h"

/** \brief This class stores the information about a root isotope.
 *
 *  It is derived publicly from class Node and differs by two members.
 *  One provides a pointer to the next root isotope in the linked list
 *  of root isotopes. The other provides a cross-referencing list of 
 *  mixtures and components where this root isotope is found.
 */

class Root : public Node
{
protected:

  /** \brief Cross referencing with the mixtures and components is done 
   *         through class MixCompRef which is defined in protected scope.  
   *
   *  This class is implemented as a linked list, where each member of 
   *  the list points to different mixture/component pair where this root 
   *  isotope is included.
   */

  class MixCompRef
    {
    protected:
      Mixture*
        /// Pointer to the mixture of this reference
        mixPtr;

      Component*
        /// Pointer to the component of this reference
        compPtr;

      double 
        /// Density of this isotope in compPtr of mixPtr
        density;
      
      MixCompRef*
        /// Next in list  
        next;

      /// This function searches the list for a reference with the same
      /// mixture and inserts a new reference after it.
      /** If none is found, a new reference is added at the end. Note that 
          this always adds a new reference - another function determines
          whether or not a new reference is needed. */
      void add(MixCompRef *);

      /// Given a mixture/component pair, find a reference in this root
      /// isotope's list that has the same combination.
	  /** Returns a pointer to the match, or NULL if none is found. */
      MixCompRef* find(MixCompRef *);

      /// Given a mixture/component pair, find a reference in this root
      /// isotope's list that has the same combination.
	  /** If the second argument is NULL, it returns the first match of the
          Mixture alone, if there is one. */
      MixCompRef* find(Mixture*,Component*);

    public:

      /// Default constructor when called with no arguments
      /** The default constructor sets the pointers to NULL and the 'density'
          to 0. Otherwise, the mixture pointer, component pointer and 
          'density' are set with arguments in that order - 'next' is always 
          NULL. */
      MixCompRef(Mixture* addMix=NULL, Component* addComp=NULL, 
		 double isoDens=0);

      /// Copy constructor
	  /** Copy constructor copies is identical to default constructor.
          Therefore, all the members are set except 'next' = NULL. */
      MixCompRef(const MixCompRef&);

      /// This constructor acts as a list insertion function.
      /** The new object copies the values of the object pointed to by the
	      first argument, and sets its 'next' pointer as the second 
          argument. */
      MixCompRef(MixCompRef*, MixCompRef*);

      /// Inline destructor destroys whole list by deleting 'next'.
      ~MixCompRef() { delete next; };

	  /// Overloaded assignment operator
	  /** The correct implementation of this operator must ensure that
          previously allocated space is returned to the free store before
          allocating new space into which to copy the object.  The 'next'
          pointer is not changed, so that this object remains a member of
          the same list as it originally was in. */
      MixCompRef& operator=(const MixCompRef&);

      /// This function merges two lists of MixCompRefs.
	  /** The list pointed to by the argument is merged into the list 
          through which the function is accessed.  Each of the items in
          the new list is searched for in the existing list.  All matches 
          are ignored and all non-matches generate a new entry in the
          existing list. */
      void tally(MixCompRef *);
      
      /// The reference flux for truncation is a group-wise maximum
      /// across the various intervals.
      /** Each root isotope has a different reference flux, based on the 
          set of intervals which contain that root.  This list of intervals in 
          which the solution for a given root should be performed is determined 
          by accessing mixture reference through this list. */
      void refFlux(Volume*);

      // NEED COMMENT Wasn't sure how to break this into brief/detailed comment 
	  /* The list of intervals in which the solution for a given root
      isotope should be performed is determined by accessing mixture
      references through this list.  The Chain and topSchedule are
      passed through this function to each of the mixtures referenced
      in this list. */
      void solve(Chain*, topSchedule*);

      /// This function simply calls Mixture::writeDump() on every
      /// mixture containing the root to which this MixCompRef belongs.
      void writeDump();

	  /// This function polls each mixture containing the root to which
      /// this MixCompRef belongs and finds the maximum relative
      /// concentration of this root in any mixture.
      double maxConc();

	  // NEED COMMENT
      double mixConc(Mixture*);

      /// This function passes its 'kza' argument on to the readDump
      /// function of each Mixture containing the root to which with
      /// MixCompRef belongs.
      void readDump(int);


      /// This function searches for a given Mixture/Component pair and
      /// finds the entry which follows it if it has the same mixture.
      /** If the third argument is NULL, it will return the first match
          of the mixture.  Once the entry is found, the first reference
          argument is set equal to the density. */
      Component* getComp(double&, Mixture*, Component*);
      
    } 
  /// This is the head of a linked list of mixture/component
  /// cross-references.
  *mixList;

  // This points to the next root isotope in the list of root isotopes.
  // This list is sorted by KZA number

  Root* nextRoot;
  
  
  /// This function adds a root isotope to the list of root isotopes.
  /** It has already been established that this isotope does not occur
      in the list, therefore, it always results in a new object.  The
      new object may be inserted at the front of the list, in the middle
      of the list or at the end of the list. */ 
  void add(Root*);
  
public:
  /// Default constructor 
  /** Invokes default constructor of base class Node, and sets 'mixList' 
      and 'next' to NULL. */
  Root();

  /// Copy constructor 
  /** Invokes copy constructor for base class Node and copies pointer 
      'mixList', ie. new mixList points to old mixList, and NOT a copy of the
	  mixList. 'next' = NULL. */
  Root(const Root&);

  /// Constructor acts as insertion function 
  /** Invoks a copy constructor of base class Node with the dereferenced
      first argument and then copying the 'mixList' pointer (see note for 
	  copy constructor above).  The 'next' pointer is set to the second
      argument. */
  Root(Root*,Root*);

  /// Constructor invokes Node(char*) constructor with first argument,
  /// initializes 'mixList' by creating a new MixCompRef object with
  /// MixCompRef(Mixture*,Component*,double), and sets 'next' to NULL.
  Root(char*,double,Mixture*,Component*);

  /// Inline destructor 
  /** Has no special actions. It does NOT delete the list of Root isotopes. */
  ~Root() 
    { delete mixList; delete nextRoot; };

  /// This inline function helps establish the reference flux by passing
  /// this Volume through 'mixList' to the list of intervals which contain
  /// a particular root isotope.
  void refFlux(Volume *refVolume)
    { mixList->refFlux(refVolume); };

  /// This function is the top level of the solution phase.
  /** For each root isotope, is creates a chain object and then loops over
      all the chains that are rooted in that isotope, solving each one with
      the created chain and the passed schedule. */
  void solve(topSchedule*);
  
  /// This function calls MixCompRef::readDump() for each root isotope
  /// in the problem.
  void readDump();

  /// This function calls MixCompRef::readDump() for the next target
  /// isotope only.
  /** It returns a pointer to that target isotope and the 'kza' value for 
      that target in the first argument. */
  Root* readSingleDump(int&);

  // NEED COMMENT
  Component* getComp(double&,Mixture*,Component*);

  /// Search through the list of root isotopes for a particular kza,
  /// passed as the argument.
  /** It returns the pointer to the matched object, or NULL if no match. */
  Root* find(int);

  /// Simple pas through to MixCompRef::maxConc(), returning its return
  /// value.
  double maxConc();

  // NEED COMMENT
  double mixConc(Mixture*);

  // NEED COMMENT
  Root* getNext() { return nextRoot; } ;
  
  /// This function merges the list pointed to by the first argument
  /// with the list of Root isotopes which it was called with.
  /** For each root in the new list, it if does not exist in the list,
      it is added with add(...).  If it does already exist, its
      mixture/component reference are added with 
	  MixCompRef::tally(...). */
  Root* merge(Root*);
  
};


#endif
