/* $Id: Volume.h,v 1.14 2002-09-09 19:57:53 varuttam Exp $ */
#include "alara.h"

#ifndef _VOLUME_H
#define _VOLUME_H

/* fineMP volume head */
#define VOL_HEAD -1

#include "Output/Result.h"

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
protected:
  double 
    /// The volume of the interval in the units of the problem.
    volume, 
    
    /// The spatial normalization of this zone.
    norm,

    /// The actual volume of the interval.
    uservol;

  char 
    /// The descriptive name of the zone in which this interval is
    /// located.
    /** This is a reference to one of the material loading objects of
        type Loading in the problem's loading list. */
    *zoneName;

  Loading* 
    /// A pointer to the Loading object referenced by 'zoneName'.
    zonePtr;

  Mixture* 
    /// A pointer to the mixture which is loaded in this invterval.
    mixPtr;

  VolFlux 
    /// The head item of a list of VolFlux objects.
    /** Each VolFlux object has an array for the flux at this interval.
        There are as many items in the list as there are defined fluxes 
        in the problem. */
    *fluxHead, 
    
    /// The tail item of a list of VolFlux objects.  
    /** This is used to help add new VolFlux items to the list. */
    *flux;

  topScheduleT* 
    /// A pointer to the storage matrix container for this interval.
    /** The transfer matrices are stored from one solution to the next 
        to minimize the recalculation of matrix elements.  Due to the
        complicated schedules and histories, a tree-type object of type
        topScheduleT (derived from calcScheduleT) is needed to store all
        the different transfer matrices. */
    schedT;

  Result 
    /// A linked list of results.
    /** Each Result item in the list points to a linked list of results 
        for a particular root isotope.  There is one Result item in the
        list for each root isotope in the interval. */
    results;

  int 
    /// The number of components in this zone.
    nComps;

  Result* 
    /// A linked list of final number densities for each component and for
    /// the total.
    outputList;

  double 
    /// The response totalled over the whole interval is stored in an 
    /// array to enable the printing of a table of totals.
    *total;

  Volume* 
    /// A pointer to the next Volume object in the problem-wide list.
    /** This pointer represents the next Volume in the sequence
	defined purely by geometry.  That is, this next Volume is
	necessarily adjacent to the current Volume. */
    next;

  Volume* 
    /// A pointer to the next Volume object in the mixture list.
    /** This pointer represents the next Volume that contains the same
	mixture as the current Volume, and comes next in the geometric
	sequence.  This next Volume is not necessarily adjacent to the
	current Volume. This is used to traverse the set of Volumes
	that contain the same mixture. */
    mixNext;

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
  /** This constructor always call init.  When called without arguments,
      the default constructor creates a blank list with no problem data.
      Otherwise, it sets the volume of the interval, and creates and
      fills the storage for 'zoneName'. */
  Volume(double vol=VOL_HEAD, char *name=NULL);

  /// Special constructor used to convert Dimension lists to Volume
  /// lists - called from Volume::calculate(...).  
  /** This is identical to default constructor, the first argument used
      as the volume and accessing the name of the zone from the second
      argument, a pointer to the zone where this interval is located.  
      The 'zonePtr' is then initialized with this second argument. */
  Volume(double, Loading*);

  /// Copy constructor
  /** This constructor starts by calling init(), and then copies the 
      'volume', 'norm', 'zonePtr', 'mixPtr', and 'zoneName' from the
      passed argument. */
  Volume(const Volume&);

  // NEED COMMENT
  Volume(Root*,topSchedule*);


  /// This destructor calls deinit(), and then destroys the whole list
  /// by deleting 'next'.
  ~Volume();

  /// Overloaded assignment operator
  /** This assignment operator first calls deinit() and init() to
      reinitialize its storage.  It then behaves similarly to the copy
      constructor.  Note that 'next' and 'mixNext' are NOT copied, the
      left hand side object will continue to be part of the same list
      unless explicitly changed. */
  Volume& operator=(const Volume&);
  
  /// This function reads a whole list of intervals from the input file
  /// attached to the passed stream reference.
  /** It extends the linked list until reaching the keyword "end". */
  void getVolumes(istream&);
  
    
  /// This function cross-checks the specified zone references to ensure 
  /// that they exist in the material loading list pointed to by the
  /// argument.
  /** When intervals are specified in the input file (as opposed to
      being calculated by the code), zones are given in the input. When 
      the referenced zone is found, the 'zonePtr' member is set. */
  void xCheck(Loading*);

  /// This function generates a new interval, calculating the volume
  /// based on the type of geometry and the zone definition.  
  /** The coordinates of the inside corner of the interval are specified 
      in the first argument with the dimensions of the interval in the
      second.  The third argument specifies the order in which the first
      two arguments should be interpreted.  The fourth argument is used
      to determine the type of geometry and the fifth for
      cross-referencing the newly generated interval with a zone. */
  Volume* calculate(double* , double* , int* , Geometry* , Loading*);

  /// This function is used to cross-reference the intervals with the
  /// list of mixtures pointed to by the first argument.  
  /** This is technically redundant once the zone's have been 
      cross-referenced, but is prevents too many look-ups later in the 
      code.  The 'mixPtr' pointer is set following a lookup on the list
      pointed to by the first argument.  The 'this' pointer is then 
      passed to the object pointed to by 'mixPtr' to reference it the 
      other way. */
  void xRef(Mixture *);

  /// This function is used to cross-reference the intervals with the
  /// list of normalizations pointed to by the first argument.
  void xRef(Norm *);

  /// This function is called as the last step of the Volume<->Mixture
  /// cross-referencing.
  /** Volume::xRef(Mixture*) passes its 'this' pointer to a Mixture 
      object which immediately passes it back to a different Volume 
      object, that mixture's volList pointer, as the base of this call.
      */
  void addMixList(Volume *);

  /// This routine reads the flux from the filename specified in the
  /// first argument.
  /** After opening the file, it skips the number of entries specified
      in the second argument, and then reads one entry for each of the
      intervals, scaling it by the third argument.  This should be called
      through the head of the list of intervals. */
  void readFlux(char *, int, double);

  /// This routine initializes a storage space hiearchy for the transfer
  /// matrices.  
  /** The first argument points to the problem schedule
      hierarchy so that the correct storage space hierarchy can be
      initialized. */
  void makeSchedTs(topSchedule*);

  /// This is the master routine for the creation of the reference flux.
  /** For each interval in the mixture list, the reference flux (part of
      the Volume pointed to by the first argument) is compared with the
      interval's flux and updated if necessary. */
  void refFlux(Volume*);

  /// This is the master routine for the solution of the chain on the
  /// master schedule.
  /** Argument 1 is the chain, and agument 2 is the master schedule.
      The chain is folded with the fluxes to get scalar rates, the 
      transfer matrices are set (which generates the solution), and the
      results are tallied. */
  void solve(Chain*, topSchedule*);

  /// This is identical to solve(...) but does not perform the tallying.
  /** Instead, the topSchedule storage pointer 'schedT' is returned in
      order to extract an array of relative productions at the various
      cooling times. */
  topScheduleT* solveRef(Chain*, topSchedule*);

  /// This short function supports the creation of the dump file.
  /** It calls writeDump() for each interval in the mixture's list of
      intervals. */
  void writeDump();

  /// This short function supports the reading of the dump file.
  /** It calls readDump() for each interval in this mixture's list of
      intervals, and then tallies those results to the various lists of
      results. */
  void readDump(int);

  /// This is the front-end to the function that tallies the results for
  /// each root isotope into lists of results for each component and a 
  /// total list of results.
  void postProc();

  /// This function is responsible for writing the results to standard
  /// output.  
  /** The first argument indicates which kind of response is being 
      written, the second indicates whether a mixture component breakdown 
      was requested, and the third points to the list of after-shutdown 
      cooling times.  The fourth argument indicates the kza of the target 
      isotope for a reverse calculation and is simply passed on the the 
      Result::write().  The final argument indicates what type of 
      normalization is being used, so that the correct output information
      can be given. */
  void write(int, int, CoolingTime*,int,int);

  /// Inline function to determine whether this object is the head of
  /// the list.
  /** Creates boolean by comparing 'volume' to VOL_HEAD. */
  int head() {return (volume == VOL_HEAD && next == NULL);};

  /// This function is used for reverse calculations to clear the values
  /// of the outputList 
  /** It does this because the results are not cummulative across subsequent
      targets. */
  void resetOutList();

  /// This function returns the number of obejcts in the linked list
  /// not including the head of the list.
  int count();

  /// This function reads values from a matrix of flux values.
  /** It scales the values by the third argument, and skips some values
      according to the second argument. */
  void storeMatrix(double** fluxMatrix, double scale);

};


#endif
