/* $Id: Dimension.h,v 1.4 2002-08-05 20:23:15 fateneja Exp $ */
#include "alara.h"

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

/** \brief This class is invoked as a linked list and describes the 
 *         geometry of the problem in a variety of co-ordinate systems.
 *     
 *  The single object of class Input has a list of Dimensions.  The 
 *  first element in each list has type DIM_HEAD and contains no problem 
 *  data.
 */

class Dimension
{
protected:
  int 
    /// This indicates the type of this dimension and is based on the
    /// definitions given below.
    type, 
    
    /// This describes the number of intervals in the whole dimension.
    /** It is the sum of the number of intervals in each zone in this
        dimension. */
    nInts, 
    
    /// This describes the number of zones in this dimension.
    nZones;

  double 
    /// This is the spatial location of the first zone boundary - the
    /// outside boundary.
    /** The units are unimportant as long as they are consistent. */
    start;

  Zone 
    /// A pointer to the first zone in the list of zones in this
    /// dimension.
    /** This pointer will be used to perform actions on the whole list,
        rather than just the last zone. */
    *zoneListHead;

  Dimension* 
    /// A pointer to the next Dimension object in the list.
    next;

public:
  /// Default constructor
  /** This constructor creates a blank list head with no arguments.
      Otherwise, it sets only the type of dimension. */
  Dimension(int dimType = DIM_HEAD);

  /// Copy constructor
  /** Copy constructor sets all scalars, but does not copy the zone 
      list or the successive dimension list. */
  Dimension(const Dimension&);

  /// Destructor destroys the whole list of dimensions by deleting next.
  /// Also deletes the zoneList in each dimension object.
  ~Dimension();

  /// Overloaded assignment operator
  /** This assignment operator functions similarly to the copy
      constructor.  The correct implementation of this operator must
      ensure that previously allocated space is returned to the free
      store before allocating new space into which to copy the object.
      It doesn't copy the pointer or the object for zoneListHead or
      next, but it does delete any previously existing zoneList
      information. */
  Dimension& operator=(const Dimension&);

  /// This function reads a single input definition for a Dimension
  /// object from the expected istream reference argument.
  /** It returns the pointer to the new object that is created.  After 
      reading which type of dimension this is, it reads the initial zone
      boundary, followed by a list of pairs: number of intervals in the
      zone and the zone boundary. */
  Dimension* getDimension(istream&);
  
  /// This funtion does some rudimentary checking for two things:
  ///   1) ensure that no dimension is repeated
  ///   2) ensure that the defined dimensions are appropriate for the
  ///      defined geometry type.
  /** For all dimensions which are not explicitly defined, it pads the
      dimension list with entries that each have one zone of unit size
      and a single interval.  The function expects an integer describing
      the type of geometry which has been defined for the problem. */
  void checkTypes(int);

  /// This function sets up the conversion from a spatial description of
  /// the various dimensions into a list of intervals.
  /** Based on the type of geometry, the dimensions are indexed with the 
      pointers to the zoneLists, to be passed to a static member function 
      of class Zone to do the final conversion.  As arguments, this function
      expects the pointer to the interval list, the pointer to the list
      of material loadings, and the pointer to the geometry description. */
  void convert(Volume*, Loading*, Geometry*);

  /// Inline access function returns the number of zones in this
  /// dimension.
  int numZones() {return nZones;};

  /// Inline access function returns the number of intervals in this
  /// dimension.
  int numInts() {return nInts;};

  /// Inline function to return the boolean result.  True if this
  /// Dimension is type DIM_HEAD.
  int head() {return (type == DIM_HEAD && next == NULL);};

  /// This function searches for a dimension of a given type and returns
  /// a pointer to it.
  /** The argument specifies the type of dimension being searched for. */
  Dimension* find(int);

  /// This function simply sets the nZones and nInts variables through
  /// calls to the appropriate function on the zoneList.
  void count();

  /// This function goes through all the dimensions and calculates the
  /// total number of zone by taking the product of all the nZones.
  int totZones();
  
};



#endif
