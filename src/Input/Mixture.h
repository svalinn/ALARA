/* $Id: Mixture.h,v 1.6 1999-08-25 15:42:51 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list and describes the various
mixtures used in the problem.  Each object is a named mixture which
may be referenced as part of zero, one or more loadings.  The first
element in each list is named IN_HEAD (defined in Input.h), and
contains no problem data.

 *** Class Members ***

 mixName : char*
    The descriptive name of the mixture is used to reference this
    Mixture from other elements of the input.

 compListHead : Component*
    A pointer to the first Component object in the list of components
    of this Mixture.  This pointer is used to initiate actions or
    perform actions on the whole list, rather than just a single
    Component object.

 volList : Volume*
    A pointer to a list of Volume objects, specifying which intervals
    contain this mixture.  Note: the Volume object has two list
    pointers, one defining the list in the order they are input and
    another defining this list of intervals which have the same
    mixture.

 rootList : Root*
    A pointer to a list of Root objects, specifying which root
    isotopes are contained in this Mixture.  This is a
    cross-referencing tool, as each root has pointers back to the
    Mixture.

 volume : double
    The volume of the mixture for weighting the tallying of the
    results.

 nComps : int
    The number of components in this zone.

 outputList : Result*
    A linked list of final number densities for each component and for
    the total.

 total : double*
    The response totalled over the whole mixture is stored in an array
    to enable the printing of a table of totals.

 next : Mixture*
    The next Mixture in the list of possible Mixtures.

 *** Member Functions ***

 * - Constructors & Destructors - *

 Mixture(char*)
    When called without arguments, the default constructor creates a
    blank list head with no problem data.  Otherwise, it creates and
    fills the storage for 'mixName', initializes a new component list
    and volume list, and sets the 'rootList' and 'next' pointer to
    NULL.

 Mixture(const Mixture&) 
    Inline copy constructor invokes default constructor.  Therefore,
    the name is copied, but the component and interval lists are
    initialized as new lists, but the rootList and successive list
    item 'next' are not copied.

 ~Mixture()
    Destructor deletes storage for 'mixName', component list
    and rootList, but not the interval list.  It then destroys the
    rest of the mixture list by deleting 'next'.
  
  Mixture& operator=(const Mixture&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the object will continue to be part of the
    same list unless explicitly changed.

 * - Input - *

 Mixture* getMixture(istream&)
    Read an entire mixture definition from the input file attached to
    the passed stream reference.  It reads each Component in the list
    up to keyword 'end', and returns a pointer to the newly created
    Mixture object.

 * - xCheck - *
 void xCheck()
    Cross-checks the input by ensuring that each named Mixture
    referenced in a component of type COMP_SIM exists in the list of
    mixtures.  If an inconsitency is found, an error results.  This
    should be called through the head of the mixture list as it
    internally loops through each of the Mixture list elements.

 * - Preproc - * 

 protected void copySim(Mixture*) 
    When the Mixture passed as the argument is referenced in a
    COMP_SIM Component, the Component is replaced with the Component
    list of the mixture definition to which it is similar.  This is
    called by removeUnused() (above) once for each Mixture.

 void removeUnused(Loading*) 
    Searches through other mixtures (by call to copySim() below) and
    material loadings to see if this mixture is used anywhere.  Once
    copySim() is complete, if a Mixture is not referenced in a
    material Loading, a warning is generated and the Mixture is
    deleted.  This loops internally through all the Mixture elements,
    and should be called through the head to the Mixture list with
    the head of the Loading list as its argument.

 void xRef(Volume*)
    Cross-references the volumes with the Mixtures by adding the
    Volume referenced in the only argument to the list of intervals.
    This is done by simply passing this reference back to a function
    of the Volume list head object, if the list has begun, otherwise,
    setting the Volume list head to point to this same object.

 void makeRootList(Root *&)

    This function should be called through the head of the Mixture
    list.  It loops through each of the Mixture objects, creating the
    rootList for each by expanding the list of Components.  As each
    rootList is created, it is merged with the top-level/master root
    list, massed by reference to point as the only argument.

 * - Solution - *

 void refFlux(VolFlux*)
    Function simply passes the argument to the list of intervals which
    contain this mixture.  The reference flux is checked and updated
    against each interval.

 void solve(Chain*, topSchedule*)
    Function simply passes the two arguments, the chain information
    and the schedule informaiton, to the list of intervals which
    contain this mixture.  The chain is solved on the schedule for
    each interval.

 * - Postproc - *

 void tally(Result*,double)
    This function tallies the result list pointed to by the first
    argument into this object's result list.  The tallying is weighted
    by the second argument.  This is used to tally the results of each
    interval in to the total mixture results, weighted by the interval
    volume.

 void write(int,int,CoolingTime*)
    This function is responsible for writing the results to standard
    output.  The first argument indicates which kind of response is
    being written, the second indicates whether a mixture component
    breakdown was requested, and the last points to the list of
    after-shutdown cooling times.

 * - Utility - *

  Component* getComp(int, double&, Component*)
     This function finds the Root object with 'kza' matching the first
     argument, and then searches that object's MixCompRef references
     for the next component of this mixture that contains this root.
     The last match is indicated by the third argument (and the 'this'
     pointer).  The reference argument (double&) is updated with the
     density of the root isotope in the appropriate component, when
     the match is found.

  int getCompNum(Component*)
     This function calls the counterpart function of class Component
     to return an ordinal number of this component in the list of
     components.

  int head() 
    Inline function to return boolean indicating if this is the head
    of the list.  Boolean generated by comparing the descriptive name,
    'mixName' to IN_HEAD.

 char* getName()
    Inline function provides access to 'mixName' string.  Note!
    Pointer is returend, so it could be changed - not safe!

 Mixture* find(char*)
    Function to search for a specific Mixture based on its descriptive
    name, passed as the only argument.  If found, a pointer to the
    appropriate Mixture object is returned, otherwise, NULL.

 int getNComps*()
    Inline function returns a count of the number of components.

 int getCompList()
    Inline function provides access to the list of components in the
    mixture.

 */


#ifndef _MIXTURE_H
#define _MIXTURE_H

#include "Input_def.h"

class Mixture
{
protected:
  char *mixName;
  Component *compListHead;
  Component *targetCompListHead;
  Volume *volList;
  Root* rootList, *targetList;
  double volume, totalDensity;
  int nComps;
  Result *outputList;
  double *total;

  Mixture *next;

  /* Preproc */
  void copySim(Mixture*);

public:
  /* Service */
  Mixture(char* name=IN_HEAD);
  Mixture(const Mixture&);
  ~Mixture();

  Mixture& operator=(const Mixture&);

  /* Input */
  Mixture* getMixture(istream&);

  /* xCheck */
  void xCheck();

  /* Preproc */
  void removeUnused(Loading*);
  void xRef(Volume*);
  void makeRootList(Root *&);

  /* Solution */
  void refFlux(VolFlux*);
  void solve(Chain*, topSchedule*);
  void writeDump();

  /* Postproc */
  void readDump(int);
  void tally(Result*,double);
  void write(int,int,CoolingTime*,int);

  /* Utility */
  Component* getComp(int, double&, Component*);
  int getCompNum(Component*);
  int head() { return (!strcmp(mixName,IN_HEAD));};
  char* getName() { return mixName; };
  Mixture* find(char*);
  int getNComps() { return nComps; };
  Component* getCompList() { return compListHead; };
  void resetOutList();
  void incTotalDensity(double incr) { totalDensity += incr; };
  double getTotalDensity() { return totalDensity; };

};



#endif



