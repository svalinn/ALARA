#include "alara.h"

/* ******* Class Description ************

This class stores the information about a root isotope.  It is derived
publicly from class Node and differs by two members.  One provides a
pointer to the next root isotope in the linked list of root isotopes.
The other provides a cross-referencing list of mixtures and components
where this root isotope is found.

 *** Locally Defined Classes ***

 MixCompRef

    Cross referencing with the mixtures and components is done through
    class MixCompRef which is defined in protected scope.  This class
    is implemented as a linked list, where each member of the list
    points to different mixture/component pair where this root isotope
    is included.

    *** Class Members ***

    mixPtr : Mixture*
       The pointer to the mixture which this reference points to.

    compPtr : Component*
       The pointer to the component which this reference points to.

    density : double
       The density of this root isotope in mixture/component pair
       which this reference points to.

    nextRoot : MixCompRef*
       The next mixture/component pair that contain this root isotope.

    *** Protected Member Functions ***

    * - Lists - *

    void add(MixCompRef*)
       This function searches the list for a reference with the same
       mixture and inserts a new reference after it.  If none is
       found, a new reference is added at the end.  Note that this
       always adds a new reference - another function determines
       whether or not a new reference is needed.

    * - Utility - *

    MixCompRef* find(MixCompRef*)
       Given a mixture/component pair, find a reference in this root
       isotope's list that has the same combination.  Returns a
       pointer to the match, or NULL if none is found.

    MixCompRef* find(Mixture*, Component*)
       Given a mixture/component pair, find a reference in this root
       isotope's list that has the same combination.  If the second
       argument is NULL, it returns the first match of the Mixture
       alone, if there is one.

    *** Public Member Function ***

    * - Constructors & Destructors - *

    MixCompRef(Mixture*, Component*, double);
       If called with no arguments, the default constructor sets the
       pointers to NULL and the 'density' to 0.  Otherwise, the
       mixture pointer, component pointer and 'density' are set with
       arguments in that order - 'next' is always NULL.

    MixCompRef(const MixCompRef&)
       Copy constructor copies is identical to default constructor.
       Therefore, all the members are set except 'next' = NULL.

    MixCompRef(MixCompRef*, MixCompRef*)
       This constructor acts as a list insertion function.  The new
       object copies the values of the object pointed to by the first
       argument, and sets its 'next' pointer as the second argument.

    ~MixCompRef()
       Inline destructor destroys whole list by deleting 'next'.

    MixCompRef& operator=(const MixCompRef&)
       The correct implementation of this operator must ensure that
       previously allocated space is returned to the free store before
       allocating new space into which to copy the object.  The 'next'
       pointer is not changed, so that this object remains a member of
       the same list as it originally was in.


    * - Lists - *

    void tally(MixCompRef *)
       This function merges two lists of MixCompRefs.  The list
       pointed to by the argument is merged into the list through
       which the function is accessed.  Each of the items in the new
       list is searched for in the existing list.  All matches are
       ignored and all non-matches generate a new entry in the
       existing list.
    
    * - Solution - *

    void refFlux(VolFlux*)
       The reference flux for truncation is a group-wise maximum
       across the various intervals.  Each root isotope has a
       different reference flux, based on the set of intervals which
       contain that root.  This list of intervals in which the
       solution for a given root should be performed is determined by
       accessing mixture reference through this list.

    void solve(Chain*, topSchedule*)
       The list of intervals in which the solution for a given root
       isotope should be performed is determined by accessing mixture
       references through this list.  The Chain and topSchedule are
       passed through this function to each of the mixtures referenced
       in this list.

    * - Postproc - *

    Component *getComp(double&, Mixture*, Component*)
       This function searches for a given Mixture/Component pair and
       finds the entry which follows it if it has the same mixture.
       If the third argument is NULL, it will return the first match
       of the mixture.  Once the entry is found, the first reference
       argument is set equal to the density.


 * END MixCompRef DESCRIPTION *


 *** Class Members ***

 mixList : MixCompRef*
    This is the head of a linked list of mixture/component
    cross-references.

 next : Root*
    This points to the next root isotope in the list of root isotopes.
    This list is sorted by KZA number.

 *** Protected Member Functions ***

 * - List - *

 void add(Root*)
    This function adds a root isotope to the list of root isotopes.
    It has already been established that this isotope does not occur
    in the list, therefore, it always results in a new object.  The
    new object may be inserted at the front of the list, in the middle
    of the list or at the end of the list.  

 *** Public Member Functions ***

 * - Constructors & Destructors - *

 Root()
    Default constructor invokes default constructor of base class
    Node, and sets 'mixList' and 'next' to NULL.

 Root(const Root&)
    Copy constructor invokes copy constructor for base class Node and
    copies pointer 'mixList', ie. new mixList points to old mixList,
    and NOT a copy of the mixList. 'next' = NULL.

 Root(Root*,Root*)
    Constructor acts as insertion function by invoking copy
    constructor of base class Node with the dereferenced first
    argument and then copying the 'mixList' pointer (see note for copy
    constructor above).  The 'next' pointer is set to the second
    argument.

 Root(char*,double,Mixture*,Component*)
    Constructor invokes Node(char*) constructor with first argument,
    initializes 'mixList' by creating a new MixCompRef object with
    MixCompRef(Mixture*,Component*,double), and sets 'next' to NULL.

 ~Root()
    Inline destructor has no special actions.  It does NOT delete the
    list of Root isotopes.
 
 * - Solution - *

 void refFlux(VolFlux*)
    This function helps establish the group-wise maximum reference
    flux by passing this flux through 'mixList' to the list of
    intervals which contain a particular root isotope.

 void solve(topSchedule*)
    This function is the top level of the solution phase.  For each
    root isotope, is creates a chain object and then loops over all
    the chains that are rooted in that isotope, solving each one with
    the created chain and the passed schedule.
 
 * - Postproc - *

 Component* getComp(double&, Mixture*,Component*)
    This function is just a gateway to the MixCompRef function of the
    same signature.

 * - Utility - *

 Root* find(int)
    Search through the list of root isotopes for a particular kza,
    passed as the argument.  It returns the pointer to the matched
    object, or NULL if no match.

 * - List - *

 Root* merge(Root*)
    This function merges the list pointed to by the first argument
    with the list of Root isotopes which it was called with.  For each
    root in the new list, it if does not exist in the list, it is
    added with add(...).  If it does already exist, its
    mixture/component reference are added with MixCompRef::tally(...).

 */

#ifndef _ROOT_H
#define _ROOT_H

#include "Node.h"

class Root : public Node
{
protected:

  class MixCompRef
    {
    protected:
      /* *next: next in list
       * *mixPtr: pointer to the mixture of this reference
       * *compPtr: pointer to the component of this reference
       * double: density of this isotope in compPtr of mixPtr 
       */
      Mixture* mixPtr;
      Component* compPtr;
      double density;
      
      MixCompRef* next;

      /* Lists */
      void add(MixCompRef *);

      /* Utility */
      MixCompRef* find(MixCompRef *);
      MixCompRef* find(Mixture*,Component*);

    public:
      /* Service */
      MixCompRef(Mixture* addMix=NULL, Component* addComp=NULL, 
		 double isoDens=0);
      MixCompRef(const MixCompRef&);
      MixCompRef(MixCompRef*, MixCompRef*);
      ~MixCompRef()
	{ delete next; };

      MixCompRef& operator=(const MixCompRef&);

      /* Lists */
      void tally(MixCompRef *);
      
      /* Solution */
      void refFlux(VolFlux *);
      void solve(Chain*, topSchedule*);
      void writeDump();

      /* Postproc */
      void readDump(int);
      Component* getComp(double&, Mixture*, Component*);
      
    } 
  *mixList;
    
    
  Root* nextRoot;
  
  
  /* List */
  void add(Root*);
  
public:
  /* Service */
  Root();
  Root(const Root&);
  Root(Root*,Root*);
  Root(char*,double,Mixture*,Component*);
  ~Root() 
    { delete mixList; delete nextRoot; };
  
  /* Solution */
  void refFlux(VolFlux *fluxHead)
    { mixList->refFlux(fluxHead); };

  void solve(topSchedule*);
  
  /* Postproc */
  void readDump();
  Root* readSingleDump(int&);
  Component* getComp(double&,Mixture*,Component*);

  /* Utility */
  Root* find(int);
  
  /* List */
  Root* merge(Root*);
  
};


#endif
