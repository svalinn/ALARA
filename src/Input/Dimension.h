#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list and describes the geometry of
the problem in a variety of co-ordinate systems.  The single object of
class Input has a list of Dimensions.  The first element in each list
has type DIM_HEAD and contains no problem data.

 *** Class Members ***

 type : int
    This indicates the type of this dimension and is based on the
    definitions given below.

 nInts : int
    This describes the number of intervals in the whole dimension.  It
    is the sum of the number of intervals in each zone in this
    dimension.

 nZones : int
    This describes the number of zones in this dimension.

 start : double
    This is the spatial location of the first zone boundary - the
    outside boundary.  The units are unimportant as long as they are
    consistent.

 zoneListHead : Zone*
    A pointer to the first zone in the list of zones in this
    dimension.  This pointer will be used to perform actions on the
    whole list, rather than just the last zone.

 *** Member Functions ***

 * - Constructors & Destructors - *

 Dimension(int)
    Default constructor creates a blank list head with no arguments.
    Otherwise, it sets only the type of dimension.

 Dimension(const Dimensiont&)
    Copy constructor sets all scalars, but does not copy the zone list
    or the successive dimension list.

 ~Dimension()
    Destructor destroys the whole list of dimensions by deleting next.
    Also deletes the zoneList in each dimension object.

 Dimension& operator=(const Dimension&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.  It doesn't
    copy the pointer or the object for zoneListHead or next.


 * - Input - *

 Dimension* getDimension(istream&)
    This function reads a single input definition for a Dimension
    object from the expected istream reference argument.  It returns
    the pointer to the new object that is created.  After reading
    which type of dimension this is, it reads the initial zone
    boundary, followed by a list of pairs: number of intervals in the
    zone and the zone boundary.

 * - xCheck - *
 void checkTypes(int)
    This funtion does some rudimentary checking for two things:
    1) ensure that no dimension is repeated
    2) ensure that the defined dimensions are appropriate for the
       defined geometry type.
    For all dimensions which are not explicitly defined, it pads the
    dimension list with entries that each have one zone of unit size
    and a single interval.  The function expects an integer describing
    the type of geometry which has been defined for the problem.


 * - Preproc - *
 void convert(Volume*, Loading*, Geometry*)
    This function sets up the conversion from a spatial description of
    the various dimensions into a list of intervals.  Based on the
    type of geometry, the dimensions are indexed with the pointers to
    the zoneLists, to be passed to a static member function of class
    Zone to do the final conversion.  As arguments, this function
    expects the pointer to the interval list, the pointer to the list
    of material loadings, and the pointer to the geometry description.

 * - Utility - *
 int numZones()
    Inline access function returns the number of zones in this
    dimension.

 int numInts()
    Inline access function returns the number of intervals in this
    dimension.

 int head()
    Inline function to return the boolean result.  True if this
    Dimension is type DIM_HEAD.

 Dimension* find(int)
    This function searches for a dimension of a given type and returns
    a pointer to it.  The argument specifies the type of dimension
    being searched for.

 void count()
    This function simply sets the nZones and nInts variables through
    calls to the appropriate function on the zoneList.

 int totZones()
    This function goes through all the dimensions and calculates the
    total number of zone by taking the product of all the nZones.
    
 */


#ifndef _DIMENSION_H
#define _DIMENSION_H

#include "Input_def.h"

/* Dimension types */
#define DIM_HEAD 0
#define DIM_X 1
#define DIM_Y 2
#define DIM_Z 3
#define DIM_R 4
#define DIM_THETA 5
#define DIM_PHI 6

class Dimension
{
protected:
  int type, nInts, nZones;
  double start;
  Zone *zoneListHead;

  Dimension* next;

public:
  /* Service */
  Dimension(int dimType = DIM_HEAD);
  Dimension(const Dimension&);
  ~Dimension();

  Dimension& operator=(const Dimension&);

  /* Input */
  Dimension* getDimension(istream&);
  
  /* xCheck */
  void checkTypes(int);

  /* Preproc */
  void convert(Volume*, Loading*, Geometry*);

  /* Utility */
  int numZones() {return nZones;};
  int numInts() {return nInts;};
  int head() {return (type == DIM_HEAD && next == NULL);};
  Dimension* find(int);
  void count();
  int totZones();
  
};



#endif
