/* $Id: Mixture.h,v 1.14 2002-08-05 20:23:17 fateneja Exp $ */
#include "alara.h"

#ifndef _MIXTURE_H
#define _MIXTURE_H

#include "Input_def.h"

/** \brief This class is invoked as a linked list and describes the 
 *         various mixtures used in the problem.  
 *   
 *  Each object is a named mixture which may be referenced as part of 
 *  zero, one or more loadings.  The first element in each list is named
 *  IN_HEAD (defined in Input.h), and contains no problem data.
 *  NOTE: The term constituent has been introduced in the user manual
 *  and user interaction to refer to the 'Components' of a 'Mixture'.
 *  By necessity, the vocabulary of the source code has not been
 *  changed, and class and variable names will refer to components.
 */

class Mixture
{
protected:
  char 
    /// The descriptive name of the mixture is used to reference this
    /// Mixture from other elements of the input.
    *mixName;

  Component 
    /// A pointer to the first Component object in the list of components
    /// of this Mixture.
    /** This pointer is used to initiate actions or perform actions on 
        the whole list, rather than just a single Component object. */
    *compListHead;

  Component 
    /// A pointer to the first Component object in the list of targets for
    /// a reverse calculation in this Mixture.  
    /** This pointer is used to initiate actions or perform actions on the
        whole list of targets, rather than just a single Component 
        object. */
    *targetCompListHead;

  Volume 
    /// A pointer to a list of Volume objects, specifying which intervals
    /// contain this mixture.  
    /** Note: the Volume object has two list pointers, one defining the 
        list in the order they are input and another defining this list of
        intervals which have the same mixture. */
    *volList;

  Root
    /// A pointer to a list of Root objects, specifying which root
    /// isotopes are contained in this Mixture.  
    /** This is a cross-referencing tool, as each root has pointers back 
        to the Mixture. */
    *rootList, 
    
    /// A pointer to a list of Root objects, specifying the target
    /// isotopes (expanded from the target components) for this Mixture.
    /** This is a cross-referencing tool, as each root has pointers back
        to the Mixture. */
    *targetList;

  double 
    /// The volume of the mixture for weighting the tallying of the
    /// results.
    volume, 
    
    /// The total mass density of this mixture may be used to normalize
    /// the results.
    totalDensity, 
    
    /// The total number density of this mixture.
    /** This number is used to determine the relative concentration of 
        initial isotopes for use with the 'impurity' input flag. */
    totalNDensity, 
    
    /// The total volume fraction of the mixture can be used to normalize
    /// the results.
    volFraction;

  int 
    /// The number of components in this zone.
    nComps;

  Result 
    /// A linked list of final number densities for each component and for
    /// the total.
    *outputList;

  double 
    /// The response totalled over the whole mixture is stored in an array
    /// to enable the printing of a table of totals.
    *total;

  double 
    /// Variable for contact dose
    *gammaAttenCoef;

  DataCache 
    /// Variable for contact dose
    doseConvCache;

  Mixture
    /// The next Mixture in the list of possible Mixtures.
    *next;

  /// When the Mixture passed as the argument is referenced in a
  /// COMP_SIM Component, the Component is replaced with the Component
  /// list of the mixture definition to which it is similar.  
  /** This is called by removeUnused() (above) once for each Mixture. */
  void copySim(Mixture*);

public:
  /// Default Constructor
  /** When called without arguments, the default constructor creates a
      blank list head with no problem data.  Otherwise, it creates and
      fills the storage for 'mixName', initializes a new component list
      and volume list, and sets the 'rootList' and 'next' pointer to
      NULL. */
  Mixture(char* name=IN_HEAD);

  /// Copy constructor 
  /** This constructor copies 'volume' and 'mixName', and initializes
      'compListHead', 'targetCompListHead', and 'volList', setting other
      variables to NULL. */
  Mixture(const Mixture&);

  /// Default destructor
  /** Destructor deletes storage for 'mixName', component list
      and rootList, but not the interval list.  It then destroys the
      rest of the mixture list by deleting 'next'. */
  ~Mixture();

  /// Overloaded assignment operator
  /** This assignment operator behaves similarly to the copy
      constructor.  The correct implementation of this operator must
      ensure that previously allocated space is returned to the free
      store before allocating new space into which to copy the
      object. Note that 'next' is NOT copied, the left-hand-side object
      will continue to be part of the same list unless explicitly
      changed. */
  Mixture& operator=(const Mixture&);

  /// This function reads an entire mixture definition from the input file 
  /// attached to the passed stream reference.
  /** It reads each Component in the list up to keyword 'end', and returns
      a pointer to the newly created Mixture object. */
  Mixture* getMixture(istream&);

  /// This function cross-checks the input
  /** It does this by ensuring that each named Mixture referenced in a 
      component of type COMP_SIM exists in the list of mixtures.  If an 
      inconsitency is found, an error results.  This should be called through
      the head of the mixture list as it internally loops through each of the
      Mixture list elements. */
  void xCheck();

  /// Searches through other mixtures (by call to copySim() below) and
  /// material loadings to see if this mixture is used anywhere.  
  /** Once copySim() is complete, if a Mixture is not referenced in a
      material Loading, a warning is generated and the Mixture is
      deleted.  This loops internally through all the Mixture elements,
      and should be called through the head to the Mixture list with
      the head of the Loading list as its argument. */
  void removeUnused(Loading*);

  /// Cross-references the volumes with the Mixtures 
  /** It does this by adding the Volume referenced in the only argument to
      the list of intervals. This is done by simply passing this reference 
      back to a function of the Volume list head object, if the list has begun, 
      otherwise, setting the Volume list head to point to this same object. */
  void xRef(Volume*);

  /// This function loops through each of the Mixture objects, creating the
  /// rootList for each by expanding the list of Components.
  /** This function should be called through the head of the Mixture
      list. As each rootList is created, it is merged with the 
      top-level/master root list, massed by reference to point as the only
      argument. */
  void makeRootList(Root *&);

  /// Function simply passes the argument to the list of intervals which
  /// contain this mixture.
  /** The reference flux contained in this volume is checked and updated 
      against each interval. */
  void refFlux(Volume*);

  /// Function simply passes the two arguments, the chain information
  /// and the schedule information, to the list of intervals which
  /// contain this mixture.  
  /** The chain is solved on the schedule for each interval. */
  void solve(Chain*, topSchedule*);

  /// Function simply calls Volume::writeDump() 
  /** This starts the process of writing the dump file using the list of 
      intervals which contain this mixture. */
  void writeDump();

  /// Function simply passes the argument to the list of intervals which
  /// contain this mixture in order to read the dump file.
  void readDump(int);

  /// This function tallies the result list pointed to by the first
  /// argument into this object's result list.
  /** The tallying is weighted by the second argument.  This is used to 
      tally the results of each interval in to the total mixture results, 
      weighted by the interval volume. */
  void tally(Result*,double);

  /// This function is responsible for writing the results to standard
  /// output.  
  /** The first argument indicates which kind of response is
      being written, the second indicates whether a mixture component
      breakdown was requested, and the third points to the list of
      after-shutdown cooling times. The fourth argument indicates the
      kza of the target isotope for a reverse calculation and is simply
      passed on the the Result::write().  The final argument indicates
      what type of normalization is being used, so that the correct
      output information can be given. */
  void write(int,int,CoolingTime*,int,int);

  // NEED COMMENT
  double getDoseConv(int, GammaSrc*);

  // NEED COMMENT
  void setGammaAttenCoef(int, ifstream&);

  /// This function finds the Root object
  /** It does this with 'kza' by matching the first argument, and then by
      searching that object's MixCompRef references for the next component of
      this mixture that contains this root. The last match is indicated by the
      third argument (and the 'this' pointer).  The reference argument 
      (double&) is updated with the density of the root isotope in the 
      appropriate component, when the match is found. */
  Component* getComp(int, double&, Component*);

  /// This function calls the counterpart function of class Component
  /// to return an ordinal number of this component in the list of
  /// components.
  int getCompNum(Component*);

  /// Inline function to return boolean indicating if this is the head
  /// of the list.
  /** Boolean generated by comparing the descriptive name, 'mixName' to 
      IN_HEAD. */
  int head() { return (!strcmp(mixName,IN_HEAD));};

  /// Inline function provides access to 'mixName' string.
  /** Note! Pointer is returend, so it could be changed - not safe! */
  char* getName() { return mixName; };

  /// Function to search for a specific Mixture based on its descriptive
  /// name.
  /** The name is passed as the only argument.  If found, a pointer to the
      appropriate Mixture object is returned, otherwise, NULL. */
  Mixture* find(char*);

  /// Inline function returns a count of the number of components.
  int getNComps() { return nComps; };

  /// Inline function provides access to the list of components in the
  /// mixture.
  Component* getCompList() { return compListHead; };

  /// This function is used for reverse calculations to clear the values
  /// of the outputList, since the results are not cummulative across
  /// subsequent targets.
  void resetOutList();

  /// Inline function provides read access to the current value of the
  /// total mass density.
  void incrTotalDensity(double incr) { totalDensity += incr; };

  /// Inline function provides read access to the current value of the
  /// total number density.
  double getTotalDensity() { return totalDensity; };

  /// Inline function provides read access to the current value of the
  /// total number density.
  void incrTotalNDensity(double incr) { totalNDensity += incr; };

  /// Inline function provides read access to the current value of the
  /// total number density.
  double getTotalNDensity() { return totalNDensity; };

  /// Inline function increments the total volume fraction by the value
  /// of the single argument.
  void incrVolFrac(double incr) { volFraction += incr; };

  /// Inline function provides read access to the current value of the
  /// total volume fraction. 
  double getVolFrac() { return volFraction; };
};

#endif



