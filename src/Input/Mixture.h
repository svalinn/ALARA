/* $Id: Mixture.h,v 1.19 2003-01-14 05:01:19 wilsonp Exp $ */
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
  /// The descriptive name of the mixture is used to reference this
  /// Mixture from other elements of the input.
  char *mixName;

  /// A pointer to the first Component object in the list of components
  /// of this Mixture.
  /** This pointer is used to initiate actions or perform actions on 
      the whole list, rather than just a single Component object. */
  Component *compListHead;
 
  /// A pointer to the first Component object in the list of targets for
  /// a reverse calculation in this Mixture.  
  /** This pointer is used to initiate actions or perform actions on the
      whole list of targets, rather than just a single Component 
      object. */
  Component *targetCompListHead;

  /// A pointer to a list of Volume objects, specifying which intervals
  /// contain this mixture.  
  /** Note: the Volume object has two list pointers, one defining the 
      list in the order they are input and another defining this list of
      intervals which have the same mixture. */
  Volume *volList;

  /// A pointer to a list of Root objects, specifying which root
  /// isotopes are contained in this Mixture.  
  /** This is a cross-referencing tool, as each root has pointers back 
      to the Mixture. */
  Root *rootList;
  
  /// A pointer to a list of Root objects, specifying the target
  /// isotopes (expanded from the target components) for this Mixture.
  /** This is a cross-referencing tool, as each root has pointers back
      to the Mixture. */
  Root *targetList;
 
  /// The volume of the mixture for weighting the tallying of the
  /// results.
  double volume;
    
  /// User defined volume for mixture.
  double userVol;

  /// The total mass density of this mixture may be used to normalize
  /// the results.
  double totalDensity; 
    
  /// The total number density of this mixture.
  /** This number is used to determine the relative concentration of 
      initial isotopes for use with the 'impurity' input flag. */
  double totalNDensity;
    
  /// The total volume fraction of the mixture can be used to normalize
  /// the results.
  double volFraction;

  /// The number of components in this zone.
  int nComps;

  /// A linked list of final number densities for each component and for
  /// the total.
  Result *outputList;

  /// The response totalled over the whole mixture is stored in an array
  /// to enable the printing of a table of totals.
  double *total;
  
  /// Variable for contact dose
  double *gammaAttenCoef;

  /// Variable for contact dose
  DataCache doseConvCache;
 
  /// The next Mixture in the list of possible Mixtures.
  Mixture *next;

  /// Stores ranges of charged particles in this mixture.
  /** Gvalues is referenced: Gvalues[Charged Particle]
      [Charged Particle Energy Group][Neutron energy group] */
  double ***Gvalues;

  /// Storage for the Gvalues
  double *GvaluesStorage;

  /// Charged Particle Ranges
  /** Referenced by: 
      problemRanges[Charged Particle][Charged Particle Energy Group] */
  double **problemRanges;

  /// Storage for charged particle ranges
  double *problemRangesStorage;

  /// When the Mixture passed as the argument is referenced in a
  /// COMP_SIM Component, the Component is replaced with the Component
  /// list of the mixture definition to which it is similar.  
  /** This is called by removeUnused() (above) once for each Mixture. */
  void copySim(Mixture*);

public:
  /// Default Constructor
  Mixture(char* name=IN_HEAD);

  /// Copy constructor 
  Mixture(const Mixture&);

  /// Default destructor
  ~Mixture();

  /// Overloaded assignment operator
  Mixture& operator=(const Mixture&);

  /// This function reads an entire mixture definition from the input file 
  /// attached to the passed stream reference.
  Mixture* getMixture(istream&);

  /// This function cross-checks the input
  void xCheck();

  /// Searches through other mixtures (by call to copySim() below) and
  /// material loadings to see if this mixture is used anywhere.  
  void removeUnused(Loading*);

  /// Cross-references the volumes with the Mixtures 
  void xRef(Volume*);

  /// This function loops through each of the Mixture objects, creating the
  /// rootList for each by expanding the list of Components.
  void makeRootList(Root *&);

  /// Function simply passes the argument to the list of intervals which
  /// contain this mixture.
  void refFlux(Volume*);

  /// Function simply passes the two arguments, the chain information
  /// and the schedule information, to the list of intervals which
  /// contain this mixture.  
  void solve(Chain*, topSchedule*);

  /// Function simply calls Volume::writeDump() 
  void writeDump();

  /// Function simply passes the argument to the list of intervals which
  /// contain this mixture in order to read the dump file.
  void readDump(int);

  /// This function tallies the result list pointed to by the first
  /// argument into this object's result list.
  void tally(Result*,double);

  /// This function is responsible for writing the results to standard
  /// output.  
  void write(int,int,CoolingTime*,int,int);

  /// Increments userVol by parameter value.
  void incrUserVol(double volumeUserVol) {userVol+=volumeUserVol;};

  // NEED COMMENT
  double getDoseConv(int, GammaSrc*);

  // NEED COMMENT
  void setGammaAttenCoef(int, ifstream&);

  /// This function finds the Root object
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

  /// Calculate charged particle ranges of this mixture
  void calcMixRange();

  /// Calculate Gvalues of this mixture
  void calcGvalues();

  /// Access function for rootList
  Root* getRootList() { return rootList; };

  /// Access function for the next mixture in the list
  Mixture *getNext() { return next; };

  /// Access function for userVol.
  double getUserVol() { return userVol; };

  /// Access function for Gvalues
  double ***getGvalues() { return Gvalues; };

};

#endif



