/* $Id: Norm.h,v 1.5 2003-01-13 04:34:58 fateneja Exp $ */
#include "alara.h"

#ifndef _NORM_H
#define _NORM_H

#define NORM_HEAD -1

/** \brief This class is invoked as a linked list and describes a spatial
 *         normalization for each of the geometrical intervals of the 
 *         problem.
 *
 *  Each Norm object should correspond to a Volume object where this
 *  normalization will be applied to the flux.  Such a normalization can
 *  be used, for example, to compensate for geometrical modelling
 *  shortcuts in the transport calculation: e.g. using a cylinder to model
 *  a torus.  The first element of the main problem-wide list has type
 *  NORM_HEAD (defined through the scale member), and contains no problem
 *  data.
 */

class Norm
{
protected:
  /// The scalar normalization of the flux at this interval.
  double scale;

  /// A pointer to the next Volume object in the problem-wide list.
  Norm* next;
  
public:
  /// Default constructor
  /** When called without arguments, the default inline constructor
      creates a blank list with no problem data.  Otherwise, it sets the
      normalization of the interval, setting 'next' to NULL. */
  Norm(double norm=NORM_HEAD)
    : scale(norm), next(NULL) {};
  
  /// Copy constructor 
  /** This constructor is identical to default constructor: 'next' =
      NULL. */
  Norm(const Norm& n)
    : scale(n.scale), next(NULL) {};
  
  /// In-line destructor destroys the whole list by deleting 'next'.
  ~Norm()
    { delete next; };

  /// Overloaded assignment operator
  /** This inline assignment operator behaves similarly to the copy
      constructor. The correct implementation of this operator must
      ensure that previously allocated space is returned to the free
      store before allocating new space into which to copy the
      object. Note that 'next' is NOT copied, the object will continue
      to be part of the same list unless explicitly changed. */
  Norm& operator=(const Norm& n)
    { if (this == &n) return *this; scale = n.scale; return *this; }

  /// This function reads a whole list of interval normalizations from
  /// the input file attached to the passed stream reference, extending
  /// the linked list until reaching the keyword "end".
  void getNorms(istream&);

  /// Inline function to determine whether this object is the head of
  /// the list.
  /** Creates boolean by comparing 'scale' to NORM_HEAD. */
  int head() {return (scale == NORM_HEAD && next == NULL);};
  
  /// Inline function provides read only interface to 'scale' member.
  double getScale() { return scale;};
  
  /// Inline function provides ability to advance through list by
  /// returning 'next'.
  Norm* advance() { return (this!=NULL)?next:(Norm*)NULL;};
};

#endif
