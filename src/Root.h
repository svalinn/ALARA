/* $Id: Root.h,v 1.11 2003-01-13 04:34:53 fateneja Exp $ */
#include "alara.h"

#ifndef ROOT_H
#define ROOT_H

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
      /// Pointer to the mixture of this reference
      Mixture* mixPtr;

      /// Pointer to the component of this reference
      Component* compPtr;

      /// Density of this isotope in compPtr of mixPtr
      double density;
      
      /// Next object in list  
      MixCompRef* next;

      /// This function searches the list for a reference with the same
      /// mixture and inserts a new reference after it.
      void add(MixCompRef *);

      /// Given a mixture/component pair, find a reference in this root
      /// isotope's list that has the same combination.
      MixCompRef* find(MixCompRef *);

      /// Given a mixture/component pair, find a reference in this root
      /// isotope's list that has the same combination.
      MixCompRef* find(Mixture*,Component*);

    public:

      /// Default constructor when called with no arguments
      MixCompRef(Mixture* addMix=NULL, Component* addComp=NULL, 
		 double isoDens=0);

      /// Copy constructor
      MixCompRef(const MixCompRef&);

      /// This constructor acts as a list insertion function.
      MixCompRef(MixCompRef*, MixCompRef*);

      /// Inline destructor destroys whole list by deleting 'next'.
      ~MixCompRef() { delete next; next=NULL; };

      /// Overloaded assignment operator
      MixCompRef& operator=(const MixCompRef&);

      /// This function merges two lists of MixCompRefs.
      void tally(MixCompRef *);
      
      /// The reference flux for truncation is a group-wise maximum
      /// across the various intervals.
      void refFlux(Volume*);

      /// solve the chain for each mixture (but not for each component)
      void solve(Chain*, topSchedule*);

      /// This function simply calls Mixture::writeDump() on every
      /// mixture containing the root to which this MixCompRef belongs.
      void writeDump();

      /// This function polls each mixture containing the root to which
      /// this MixCompRef belongs and finds the maximum relative
      /// concentration of this root in any mixture.
      double maxConc();

      /// search list of mixtures and find total density across all components
      double mixConc(Mixture*);

      /// This function passes its 'kza' argument on to the readDump
      /// function of each Mixture containing the root to which with
      /// MixCompRef belongs.
      void readDump(int);


      /// This function searches for a given Mixture/Component pair and
      /// finds the entry which follows it if it has the same mixture.
      Component* getComp(double&, Mixture*, Component*);
      
    } 
  /// This is the head of a linked list of mixture/component
  /// cross-references.
  *mixList;

  // This points to the next root isotope in the list of root isotopes.
  // This list is sorted by KZA number
  Root* nextRoot;
   
  /// This function adds a root isotope to the list of root isotopes.
  void add(Root*);
  
public:
  /// Default constructor 
  Root();

  /// Copy constructor 
  Root(const Root&);

  /// Constructor acts as insertion function 
  Root(Root*,Root*);

  /// Constructor invokes Node(char*) constructor with first argument,
  /// initializes 'mixList' by creating a new MixCompRef object with
  /// MixCompRef(Mixture*,Component*,double), and sets 'next' to NULL.
  Root(char*,double,Mixture*,Component*);

  /// Inline destructor 
  /** Has no special actions. It does NOT delete the list of Root isotopes. */
  ~Root() 
    { delete mixList; delete nextRoot; nextRoot=NULL;};

  /// This inline function helps establish the reference flux by passing
  /// this Volume through 'mixList' to the list of intervals which contain
  /// a particular root isotope.
  void refFlux(Volume *refVolume)
    { mixList->refFlux(refVolume); };

  /// This function is the top level of the solution phase.
  void solve(topSchedule*);
  
  /// This function calls MixCompRef::readDump() for each root isotope
  /// in the problem.
  void readDump();

  /// This function calls MixCompRef::readDump() for the next target
  /// isotope only.
  Root* readSingleDump(int&);

  // NEED COMMENT
  Component* getComp(double&,Mixture*,Component*);

  /// Search through the list of root isotopes for a particular kza,
  /// passed as the argument.
  Root* find(int);

  /// Simple pas through to MixCompRef::maxConc(), returning its return
  /// value.
  double maxConc();

  /// Search list of mixtures to find all components for this mixture and sum
  /// their densities
  double mixConc(Mixture*);

  // NEED COMMENT
  Root* getNext() { return nextRoot; } ;
  
  /// This function merges the list pointed to by the first argument
  /// with the list of Root isotopes which it was called with.
  Root* merge(Root*);
};


#endif
