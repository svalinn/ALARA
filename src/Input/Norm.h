/* $Id: Norm.h,v 1.2 1999-08-24 22:06:22 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************
   
This class is invoked as a linked list and describes a spatial
normalization for each of the geometrical intervals of the problem.
Each Norm object should correspond to a Volume object where this
normalization will be applied to the flux.  Such a normalization can
be used, for example, to compensate for geometrical modelling
shortcuts in the transport calculation: e.g. using a cylinder to model
a torus.  The first element of the main problem-wide list has type
NORM_HEAD (defined through the scale member), and contains no problem
data.

 *** Class Members ***
 
 scale : double
    The scalar normalization of the flux at this interval.


 next : Norm*
    A pointer to the next Volume object in the problem-wide list.

 *** Member Functions ***

 * - Constructors & Destructors - *

 Norm(double)
    When called without arguments, the default inline constructor
    creates a blank list with no problem data.  Otherwise, it sets the
    normalization of the interval, setting 'next' to NULL.

 Norm(const Norm&)
    Copy constructor is identical to default constructor: 'next' =
    NULL.

 ~Norm() 
    In-line destructor destroys the whole list by deleting 'next'.

 Norm& operator=(const Norm&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the object will continue to be part of the
    same list unless explicitly changed.


 * - Input - *

 void getNorms(istream&)
    This function reads a whole list of interval normalizations from
    the input file attached to the passed stream reference, extending
    the linked list until reaching the keyword "end".

 * - Utility - *

 int head()
    Inline function to determine whether this object is the head of
    the list.  Creates boolean by comparing 'volume' to VOL_HEAD.

 double getScale()
    Inline function provides read only interface to 'scale' member.

 Norm* advance()
    Inline function provides ability to advance through list by
    returning 'next'.

 */

#ifndef _NORM_H
#define _NORM_H

/* fineMP volume head */
#define NORM_HEAD -1

class Norm
{
protected:
  double scale;

  Norm* next;
  
public:
  /* Service */
  Norm(double norm=NORM_HEAD)
    { scale=norm; next=NULL; };
  Norm(const Norm& n)
    { scale=n.scale; next=NULL; };
  ~Norm()
    { delete next; };

  Norm& operator=(const Norm& n)
    { if (this == &n) return *this; scale = n.scale; return *this; }

  /* Input */
  void getNorms(istream&);

  /* Utility */
  int head() {return (scale == NORM_HEAD && next == NULL);};
  double getScale() { return scale;};
  Norm* advance() { return (this!=NULL)?next:(Norm*)NULL;};

};


#endif
