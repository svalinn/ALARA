/* $Id: Volume.h,v 1.21 2004-07-29 19:55:01 wilsonp Exp $ */
#include "alara.h"

#ifndef VOLUME_H
#define VOLUME_H

/* fineMP volume head */
#define VOL_HEAD -1

#include "Result.h"

/** \brief This class is invoked as a linked list and describes the 
 *         various geometrical intervals of the problem.  
 *
 *  Each Volume object is a member of two lists: the first is 
 *  problem-wide (a member of an object of class Input) and has the 
 *  Volume objects in the order they were input or created 
 *  (see Dimension::convert()), and the second is a list of intervals
 *  for each mixture.  The first element of the main problem-wide list
 *  has type VOL_HEAD (defined through the volume member), and contains
 *  no problem data.
 */

class Volume
{
private:
  /// Array of int contained requested interval to be summed for 
  /// adjoint dose output
  int* intervalptr;
protected:
  /// The volume of the interval in the units of the problem.
  double volume;
    
  /// The spatial normalization of this zone.
  double norm;

  /// The actual volume of the interval.
  double userVol;

  /// The descriptive name of the zone in which this interval is
  /// located.
  /** This is a reference to one of the material loading objects of
      type Loading in the problem's loading list. */
  char *zoneName;

  /// A pointer to the Loading object referenced by 'zoneName'.
  Loading* zonePtr;

  /// A pointer to the mixture which is loaded in this invterval.
  Mixture* mixPtr;

  /// The head item of a list of VolFlux objects.
  /** Each VolFlux object has an array for the flux at this interval.
      There are as many items in the list as there are defined fluxes 
      in the problem. */
  VolFlux *fluxHead;
  
  /// The tail item of a list of VolFlux objects.  
  /** This is used to help add new VolFlux items to the list. */
  VolFlux *flux;

  /// A pointer to the storage matrix container for this interval.
  /** The transfer matrices are stored from one solution to the next 
      to minimize the recalculation of matrix elements.  Due to the
      complicated schedules and histories, a tree-type object of type
      topScheduleT (derived from calcScheduleT) is needed to store all
      the different transfer matrices. */
  topScheduleT* schedT;

  /// A linked list of results.
  /** Each Result item in the list points to a linked list of results 
      for a particular root isotope.  There is one Result item in the
      list for each root isotope in the interval. */
  Result results;

  /// The number of components in this zone.
  int nComps;

  /// A linked list of final number densities for each component and for
  /// the total.
  Result* outputList;

  /// The response totalled over the whole interval is stored in an 
  /// array to enable the printing of a table of totals.
  double *total;

  /// Gamma source to dose conversion factor (adjoint calculation)
  double *adjConv;

  /// cache the isotopic specific dose contribution from this volume
  DataCache doseContrib;
 
  /// A pointer to the next Volume object in the problem-wide list.
  /** This pointer represents the next Volume in the sequence
      defined purely by geometry.  That is, this next Volume is
      necessarily adjacent to the current Volume. */
  Volume* next;
 
  /// A pointer to the next Volume object in the mixture list.
  /** This pointer represents the next Volume that contains the same
      mixture as the current Volume, and comes next in the geometric
      sequence.  This next Volume is not necessarily adjacent to the
      current Volume. This is used to traverse the set of Volumes
      that contain the same mixture. */
  Volume* mixNext;

  /// Library of Charged Particle Ranges
  static TempLibType rangeLib;
  
  /// Library of charged particle spectra
  static TempLibType specLib;

  /// Relationship between standard neutron energy groups, and the specLib
  /// neutron energy groups
  static int *energyRel;
 
  /// This function is called by many of the constructors, as it sets up
  /// all the variables, particularly setting pointers to NULL and
  /// initializing the 'flux' list.
  void init();
  
  /// This function is called by the destructor and the assignment
  /// operator to delete the storage for 'zoneName', delete the
  /// 'flux' and 'results' lists, and delete the 'schedT' storage.
  void deinit();
  
public:
  /// Default constructor
  Volume(double vol=VOL_HEAD, const char *name=NULL);

  /// Special constructor used to convert Dimension lists to Volume
  /// lists - called from Volume::calculate(...).  
  Volume(double, Loading*);

  /// Copy constructor
  Volume(const Volume&);

  // NEED COMMENT
  Volume(Root*,topSchedule*);

  /// This destructor calls deinit(), and then destroys the whole list
  /// by deleting 'next'.
  ~Volume();

  /// Overloaded assignment operator
  Volume& operator=(const Volume&);
  
  /// This function reads a whole list of intervals from the input file
  /// attached to the passed stream reference.
  void getVolumes(istream&);
  
    
  /// This function cross-checks the specified zone references to ensure 
  /// that they exist in the material loading list pointed to by the
  /// argument.
  void xCheck(Loading*);

  /// This function generates a new interval, calculating the volume
  /// based on the type of geometry and the zone definition.  
  Volume* calculate(double* , double* , int* , Geometry* , Loading*);

  /// This function is used to cross-reference the intervals with the
  /// list of mixtures pointed to by the first argument.  
  void xRef(Mixture *);

  /// This function is used to cross-reference the intervals with the
  /// list of normalizations pointed to by the first argument.
  void xRef(Norm *);

  /// This function is called as the last step of the Volume<->Mixture
  /// cross-referencing.
  void addMixList(Volume *);

  /// This routine reads the flux from the filename specified in the
  /// first argument.
  void readFlux(char *, int, double);

  /// This routine initializes a storage space hiearchy for the transfer
  /// matrices.  
  void makeSchedTs(topSchedule*);

  /// This is the master routine for the creation of the reference flux.
  void refFlux(Volume*);

  /// This is the master routine for the solution of the chain on the
  /// master schedule.
  void solve(Chain*, topSchedule*);

  /// This is identical to solve(...) but does not perform the tallying.
  topScheduleT* solveRef(Chain*, topSchedule*);

  /// This short function supports the creation of the dump file.
  void writeDump();

  /// This short function supports the reading of the dump file.
  void readDump(int);

  /// This is the front-end to the function that tallies the results for
  /// each root isotope into lists of results for each component and a 
  /// total list of results.
  void postProc();

  /// This function is responsible for writing the results to standard
  /// output.  
  void write(int, int, CoolingTime*,int,int);

  /// Inline function to determine whether this object is the head of
  /// the list.
  /** Creates boolean by comparing 'volume' to VOL_HEAD. */
  int head() {return (volume == VOL_HEAD && next == NULL);};

  /// This function is used for reverse calculations to clear the values
  /// of the outputList 
  void resetOutList();

  /// This function returns the number of obejcts in the linked list
  /// not including the head of the list.
  int count();

  /// This function reads values from a matrix of flux values.
  void storeMatrix(double** fluxMatrix, double scale);

  /// Calculate charged particle flux 
  void makeXFlux(Mixture *mixListHead);

  /// Loads the charged particle spectrum into specLib
  static void loadSpecLib(istream *probInput);

  /// Loads the charged particle ranges into rangeLib
  static void loadRangeLib(istream *probInput);

  void readAdjDoseData(int, ifstream&);
  /// This function reads adjoint field data from input file.

  double getAdjDoseConv(int, GammaSrc*);
  
  inline double getUserVol() { return userVol; }
  /// Access function for userVol

  /// Access function for specLib
  static TempLibType getSpecLib() { return specLib; };

  /// Access function for rangeLib
  static TempLibType getRangeLib() { return rangeLib; };

  /// Access function for energyRel
  static int* getEnergyRel() { return energyRel; };

  /// Write function for intervalptr
  void setintervalptr(int* pvalue) {intervalptr = pvalue;}; 
};


#endif
