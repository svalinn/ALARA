/* $Id: Volume.h,v 1.10 2000-01-17 16:57:38 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list and describes the various
geometrical intervals of the problem.  Each Volume object is a member
of two lists: the first is problem-wide (a member of an object of
class Input) and has the Volume objects in the order they were input
or created (see Dimension::convert()), and the second is a list of
intervals for each mixture.  The first element of the main
problem-wide list has type VOL_HEAD (defined through the volume
member), and contains no problem data.

 *** Class Members ***
 
 volume : double
    The volume of the interval in the units of the problem.

 norm : double
    The spatial normalization of this zone.

 zoneName : char*
    The descriptive name of the zone in which this interval is
    located.  This is a reference to one of the material loading
    objects of type Loading in the problem's loading list.

 zonePtr : Loading*
    A pointer to the Loading object referenced by 'zoneName'.

 mixPtr : Mixture*
    A pointer to the mixture which is loaded in this invterval.

 fluxHead : VolFlux*
    The head item of a list of VolFlux objects.  Each VolFlux object
    has an array for the flux at this interval.  There are as many
    items in the list as there are defined fluxes in the problem.

 flux : VolFlux*
    The tail item of a list of VolFlux objects.  This is used to help
    add new VolFlux items to the list.

 schedT : topScheduleT*
    A pointer to the storage matrix container for this interval.  The
    transfer matrices are stored from one solution to the next to
    minimize the recalculation of matrix elements.  Due to the
    complicated schedules and histories, a tree-type object of type
    topScheduleT (derived from calcScheduleT) is needed to store all
    the different transfer matrices.

 results : Result 
    A linked list of results.  Each Result item in the list points
    to a linked list of results for a particular root isotope.  There
    is one Result item in the list for each root isotope in the
    interval.

 nComps : int
    The number of components in this zone.

 outputList : Result*
    A linked list of final number densities for each component and for
    the total.

 total : double*
    The response totalled over the whole interval is stored in an array to
    enable the printing of a table of totals.

 next : Volume*
    A pointer to the next Volume object in the problem-wide list.

 mixNext : Volume*
    A pointer to the next Volume object in the mixture list.
 
 *** Member Functions ***

 * - Constructors & Destructors - *

 private init()
    This function is called by many of the constructors, as it sets up
    all the variables, particularly setting pointers to NULL and
    initializing the 'flux' list.

 private deinit()
    This function is called by the destructor and the assignment
    operator to delete the storage for 'zoneName', delete the
    'flux' and 'results' lists, and delete the 'schedT' storage.

 Volume(double, char* )
    This constructor always call init.  When called without arguments,
    the default constructor creates a blank list with no problem data.
    Otherwise, it sets the volume of the interval, and creates and
    fills the storage for 'zoneName'.

 Volume(double, Loading*) 
    Special constructor used to convert Dimension lists to Volume
    lists - called from Volume::calculate(...).  This is identical to
    default constructor, the first argument used as the volume and
    accessing the name of the zone from the second argument, a pointer
    to the zone where this interval is located.  The 'zonePtr' is then
    initialized with this second argument.

 Volume(const Volume&)
    This copy constructor starts by calling init(), and then copies
    the 'volume', 'norm', 'zonePtr', 'mixPtr', and 'zoneName' from the
    passed argument. 

 ~Volume() 
    This destructor calls deinit(), and then destroys the whole list
    by deleting 'next'.

 Volume& operator=(const Volume&)
    This assignment operator first calls deinit() and init() to
    reinitialize its storage.  It then behaves similarly to the copy
    constructor.  Note that 'next' and 'mixNext' are NOT copied, the
    left hand side object will continue to be part of the same list
    unless explicitly changed.

 * - Input - *

 void getVolumes(istream&)
    This function reads a whole list of intervals from the input file
    attached to the passed stream reference, extending the linked list
    until reaching the keyword "end".

 * - xCheck - *

 void xCheck(Loading*)
    When intervals are specified in the input file (as opposed to
    being calculated by the code), zones are given in the input.  This
    function cross-checks the specified zone references to ensure that
    they exist in the material loading list pointed to by the
    argument.  When the referenced zone is found, the 'zonePtr' member
    is set.

 * - Preproc - *

 Volume* calculate(double* , double* , int* , Geometry* , Loading*)
    This function generates a new interval, calculating the volume
    based on the type of geometry and the zone definition.  The
    coordinates of the inside corner of the interval are specified in
    the first argument with the dimensions of the interval in the
    second.  The third argument specifies the order in which the first
    two arguments should be interpreted.  The fourth argument is used
    to determine the type of geometry and the fifth for
    cross-referencing the newly generated interval with a zone.

 void xRef(Mixture *)
    This function is used to cross-reference the intervals with the
    list of mixtures pointed to by the first argument.  This is
    technically redundant once the zone's have been cross-referenced,
    but is prevents too many look-ups later in the code.  The 'mixPtr'
    pointer is set following a lookup on the list pointed to by the
    first argument.  The 'this' pointer is then passed to the object
    pointed to by 'mixPtr' to reference it the other way.

 void xRef(Norm *)
    This function is used to cross-reference the intervals with the
    list of normalizations pointed to by the first argument.

 void addMixList(Volume *)
    This function is called as the last step of the Volume<->Mixture
    cross-referencing.  Volume::xRef(Mixture*) passes its 'this'
    pointer to a Mixture object which immediately passes it back to a
    different Volume object, that mixture's volList pointer, as the
    base of this call.

 void readFlux(char *, int, double)
    This routine reads the flux from the filename specified in the
    first argument.  After opening the file, it skips the number of
    entries specified in the second argument, and then reads one entry
    for each of the intervals, scaling it by the third argument.  This
    should be called through the head of the list of intervals.

 void makeSchedTs(topSchedule*)
    This routine initializes a storage space hiearchy for the transfer
    matrices.  The first argument points to the problem schedule
    hierarchy so that the correct storage space hierarchy can be
    initialized.

 * - Solution - *

 void refFlux(VolFlux*)
    This is the master routine for the creation of the reference flux.
    For each interval in the mixture list, the reference flux (first
    argument) is compared with the interval's flux and updated if
    necessary.

 void solve(Chain*, topSchedule*)
    This is the master routine for the solution of the chain (argument
    1) on the master schedule (argument 2).  The chain is folded with
    the fluxes to get scalar rates, the transfer matrices are set
    (which generates the solution), and the results are tallied.

 topScheduleT* solveRef(Chain*,topSchedule*)
    This is identical to solve(...) but does not perform the tallying.
    Instead, the topSchedule storage pointer 'schedT' is returned in
    order to extract an array of relative productions at the various
    cooling times.

 void writeDump()
    This short function supports the creation of the dump file by
    calling writeDump() for each interval in the mixture's list of
    intervals.

 * - Postproc - *

 void readDump()
    This short function supports the reading of the dump file by
    calling readDump() for each interval in this mixture's list of
    intervals, and then tallying those results to the various lists of
    results.

 void postProc()
    This is the front-end to the function which tallies that tallies
    the results for each root isotope into lists of results for each
    component and a total list of results.

 void write(int,int,CoolingTime*,int,int)
    This function is responsible for writing the results to standard
    output.  The first argument indicates which kind of response is
    being written, the second indicates whether a mixture component
    breakdown was requested, and the third points to the list of
    after-shutdown cooling times.  The fourth argument indicates the
    kza of the target isotope for a reverse calculation and is simply
    passed on the the Result::write().  The final argument indicates
    what type of normalization is being used, so that the correct
    output information can be given.

 * - Utility - *

 int head()
    Inline function to determine whether this object is the head of
    the list.  Creates boolean by comparing 'volume' to VOL_HEAD.

 void resetOutList()
    This function is used for reverse calculations to clear the values
    of the outputList, since the results are not cummulative across
    subsequent targets.

 */

#ifndef _VOLUME_H
#define _VOLUME_H

/* fineMP volume head */
#define VOL_HEAD -1

#include "Output/Result.h"

class Volume
{
protected:
  double volume, norm;
  char *zoneName;
  Loading* zonePtr;
  Mixture* mixPtr;
  VolFlux *fluxHead, *flux;
  topScheduleT* schedT;
  Result results;
  int nComps;
  Result* outputList;
  double *total;

  Volume* next;
  Volume* mixNext;

  void init();
  void deinit();
  
public:
  /* Service */
  Volume(double vol=VOL_HEAD, char *name=NULL);
  Volume(double, Loading*);
  Volume(const Volume&);
  Volume(Root*,topSchedule*);
  ~Volume();

  Volume& operator=(const Volume&);
  
  /* Input */
  void getVolumes(istream&);
  /* xCheck */
  void xCheck(Loading*);

  /* Preproc */
  Volume* calculate(double* , double* , int* , Geometry* , Loading*);

  void xRef(Mixture *);
  void xRef(Norm *);
  void addMixList(Volume *);
  void readFlux(char *, int, double);
  void makeSchedTs(topSchedule*);

  /* Solution */
  void refFlux(VolFlux*);
  void solve(Chain*, topSchedule*);
  topScheduleT* solveRef(Chain*, topSchedule*);
  void writeDump();

  /* Postproc */
  void readDump(int);
  void postProc();
  void write(int, int, CoolingTime*,int,int);

  /* Utility */
  int head() {return (volume == VOL_HEAD && next == NULL);};
  void resetOutList();

};


#endif
