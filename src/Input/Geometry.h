/* $Id: Geometry.h,v 1.4 2003-01-13 04:34:56 fateneja Exp $ */
#include "alara.h"

#ifndef _GEOMETRY_H
#define _GEOMETRY_H

/* Geometry types */
#define GEOM_P  0 
#define GEOM_R  1
#define GEOM_C  2
#define GEOM_S  3
#define GEOM_T  4

/** \brief This class is a simple container for information about the 
 *         type of geometry.
 *
 *  Note: no copy constructor is defined because memberwise copying is
 *        appropriate.
 */

class Geometry
{
protected:
  /// This indicates the geometry type of this problem and is based on
  /// the definitions given below.
  int type;

  /// The minor radius of a torroidal configuraiton.
  double rMin;
    
  /// The major radius of a torroidal configuraiton.
  double rMaj;

public:
  /// Default constructor defaults to a point geometry if no argument 
  /// is given.
  Geometry(char *token="p");

  /// Default destructor
  ~Geometry() {};
  
  /// Inline access function to set the major radii
  void setMajorR(double majR) {rMaj = majR;};
  
  /// Inline access function to set the minor radii
  void setMinorR(double minR) {rMin = minR;};

  /// Check that the major radius is defined when torroidal geometry is
  /// specified. Also confirm that either minor radius or a minor
  /// radius dimension is defined.
  void checkTorus(Dimension*);

  /// Inline access function to major radius' value.
  double majR() {return rMaj;};

  /// Inline access function to geometry type.
  int getType() {return type;};
};

#endif
