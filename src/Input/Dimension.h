/* $Id: Dimension.h,v 1.5 2003-01-13 04:34:55 fateneja Exp $ */
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
  /// This indicates the type of this dimension and is based on the
  /// definitions given below.
  int type;
    
  /// This describes the number of intervals in the whole dimension.
  /** It is the sum of the number of intervals in each zone in this
      dimension. */
  int nInts; 
    
  /// This describes the number of zones in this dimension.
  int nZones;
 
  /// This is the spatial location of the first zone boundary - the
  /// outside boundary.
  /** The units are unimportant as long as they are consistent. */
  double start;

  /// A pointer to the first zone in the list of zones in this
  /// dimension.
  /** This pointer will be used to perform actions on the whole list,
      rather than just the last zone. */
  Zone *zoneListHead;

  /// A pointer to the next Dimension object in the list.
  Dimension* next;

public:
  /// Default constructor
  Dimension(int dimType = DIM_HEAD);

  /// Copy constructor
  Dimension(const Dimension&);

  /// Destructor destroys the whole list of dimensions by deleting next.
  /// Also deletes the zoneList in each dimension object.
  ~Dimension();

  /// Overloaded assignment operator
  Dimension& operator=(const Dimension&);

  /// This function reads a single input definition for a Dimension
  /// object from the expected istream reference argument.
  Dimension* getDimension(istream&);
  
  /// This funtion does some rudimentary checking for two things:
  ///   1) ensure that no dimension is repeated
  ///   2) ensure that the defined dimensions are appropriate for the
  ///      defined geometry type.
  void checkTypes(int);

  /// This function sets up the conversion from a spatial description of
  /// the various dimensions into a list of intervals.
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
  Dimension* find(int);

  /// This function simply sets the nZones and nInts variables through
  /// calls to the appropriate function on the zoneList.
  void count();

  /// This function goes through all the dimensions and calculates the
  /// total number of zone by taking the product of all the nZones.
  int totZones();
  
};



#endif
