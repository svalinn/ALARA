#include "alara.h"

/******* Class Description ***************

This class is a simple container for information about the type of geometry.

 *** Class Members ***

 type : int

    This indicates the geometry type of this problem and is based on
    the definitions given below.

 rMin : double
    The minor radius of a torroidal configuraiton.

 rMaj : double
    The major radius of a torroidal configuraiton.

 *** Member Functions ***

 * - Constructors & Destructors - *

 Geometry(char*)
    Default constructor defaults to a point geometry if no argument is given.

 ~Geometry()
    Default destructor;

  Note: no copy constructor is defined because memberwise copying is
        appropriate.
  
  * - Input - *

  double setMajorR(double)
  double setMinorR(double)
     Inline access functions set the major and minor radii, respectively.

  * - xCheck - *

  void checkTorus(Dimension*) 
     Check that the major radius is defined when torroidal geometry is
     specified.  Also confirm that either minor radius or a minor
     radius dimension is defined.
  

  * - Utility - *

  double majR()
     Inline access function to major radius' value.

  int getType()
     Inline access function to geometry type.

 */

#ifndef _GEOMETRY_H
#define _GEOMETRY_H

/* Geometry types */
#define GEOM_P  0 
#define GEOM_R  1
#define GEOM_C  2
#define GEOM_S  3
#define GEOM_T  4

class Geometry
{
protected:
  int type;
  double rMin,rMaj;

public:
  /* Service */
  Geometry(char *token="p");
  ~Geometry() {};
  
  /* Input */
  void setMajorR(double majR) {rMaj = majR;};
  void setMinorR(double minR) {rMin = minR;};

  /* xCheck */
  void checkTorus(Dimension*);

  /* Utility */
  double majR() {return rMaj;};
  int getType() {return type;};
};



#endif
