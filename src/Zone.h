/* $Id: Zone.h,v 1.5 2003-01-13 04:35:00 fateneja Exp $ */
#include "alara.h"

#ifndef ZONE_H
#define ZONE_H

/* zone head*/
#define ZONE_HEAD -1

/** \brief This class is invoked as a linked list, with each object of 
 *         class Dimension having a distinct list.  
 *         
 *  A full list describes all the zone boundaries and the number of
 *  intervals per zone in that dimension. The first element in each 
 *  list has type ZONE_HEAD, and contains no problem data.
 */


class Zone
{
protected:
  /// The spatial location of the zone boundary.
  double boundary;

  /// The number of intervals between the previous zone boundary and
  /// this one.
  int nInts;
 
  /// The pointer to the next Zone object in the linked list.
  Zone *next;

public:
  /// This function loops through all the intervals in a zone and 
  /// calls Volume::convert(...).
  static void convert(double*, int*, Zone**, Geometry*, 
			   Loading*, Volume*);

  /// Default constructor
  /** This constructor creates a blank list head, when no arguments
      are given.  Otherwise, it sets the boundary and number of intervals.
      The 'next' element is initialized to NULL. */
  Zone(double bound=ZONE_HEAD,int ints=0)
    : boundary(bound), nInts(ints), next(NULL) {};
  
  /// Copy constructor
  /** This constructor is identical to default constructor.  Therefore,
      the boundary and number of intervals are copied, but the
      successive component in the list ('next') is not. */
  Zone(const Zone &z)
    : boundary(z.boundary), nInts(z.nInts), next(NULL) {};
  
  /// Inline destructor destroys whole chain by deleting 'next'.
  ~Zone() 
    { delete next; next=NULL; };

  /// Overloaded assignment operator
  Zone& operator=(const Zone&);
  
  /// This function simply adds a new zone to the list by calling the
  /// constructor with the passed argument nInts and boundary.  
  Zone* addZone(int,double);
  
  /// Inline function to determine whether this object is the head of
  /// the list.  Creates boolean by comparing 'boundary' to ZONE_HEAD.
  int head() {return (boundary == ZONE_HEAD);};
  
  /// Function counts the number of zones in this linked list of zones.
  int numZones();
  
  /// Function counts the total number of intervals in this dimension.
  int numInts();

};

#endif
