/* $Id: Zone.h,v 1.3 2000-01-17 16:57:38 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list, with each object of class
Dimension having a distinct list.  A full list describes all the zone
boundaries and the number of intervals per zone in that dimension.
The first element in each list has type ZONE_HEAD, and contains no
problem data.

 *** Class Members ***

 boundary : double
    The spatial location of the zone boundary.

 nInts : int
    The number of intervals between the previous zone boundary and
    this one.

 next : Zone*
    The pointer to the next Zone object in the linked list.

 *** Static Member Functions ***

 Volume* convert(double*, int*, Zone**, Geometry*, Loading*, Volume*)
    Following setup for calculation of interval volumes in
    Dimension::convert(...), this funciton loops through all the
    intervals in a zone and calls Volume::convert(...) which actually
    calculates the volume and generates the Volume object.  The
    arguments are, respsectively, the coordinates of the starting
    point, the ordering of the coordinates, the pointers to the
    appropriate zoneLists, the problem geometry, the list of problem
    material loadings and the problem's list of intervals.

 *** Member Functions ***

 * - Constructors & Destructors - *

 Zone(double,int)
    Default constructor creates a blank list head, when no arguments
    are given.  Otherwise, it sets the boundary and number of intervals.
    The 'next' element is initialized to NULL.

 Zone(const Zone& z)
    Copy constructor is identical to default constructor.  Therefore,
    the boundary and number of intervals are copied, but the
    successive component in the list ('next') is not.

 ~Zone()
    Inline destructor destroys whole chain by deleting 'next'.

 Zone& operator=(const Zone&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.  It does NOT
    copy 'next'.


 * - Input - *

 Zone* addZone(int,double) 
    This function simply adds a new zone to the list by calling the
    constructor with the passed argument nInts and boundary.  It
    returns a pointer to the newly created zone.

 int head()
    Inline function to determine whether this object is the head of
    the list.  Creates boolean by comparing 'boundary' to ZONE_HEAD.

 int numZones()
    Function counts the number of zones in this linked list of zones.

 int numInts()
    Function counts the total number of intervals in this dimension.

 */


#ifndef _ZONE_H
#define _ZONE_H

/* zone head*/
#define ZONE_HEAD -1

class Zone
{
protected:
  double boundary;
  int nInts;

  Zone *next;

public:
  static void convert(double*, int*, Zone**, Geometry*, 
			   Loading*, Volume*);

  Zone(double bound=ZONE_HEAD,int ints=0)
    : boundary(bound), nInts(ints), next(NULL) {};
  Zone(const Zone &z)
    : boundary(z.boundary), nInts(z.nInts), next(NULL) {};
  ~Zone() 
    { delete next; };

  Zone& operator=(const Zone&);
  
  Zone* addZone(int,double);
  int head() {return (boundary == ZONE_HEAD);};
  int numZones();
  int numInts();

};




#endif
