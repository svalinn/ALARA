/* $Id: VolFlux.h,v 1.6 2000-01-23 01:07:33 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class stores the information about the neutron fluxes in each
interval.  Each Volume object will have a linked list of these VolFlux
objects, with each element in the list representing a different flux
specification.

 *** Static Class Members ***

 nFluxes : int
    The number of user-defined Flux specifications.

 nGroups : int
    The number of neutron groups being used in this problem.

 *** Non-Static Class Members ***
 
 flux : double*
    An array of scalar flux values.

 next : VolFlux*
    A pointer to the next VolFlux object in this list.

 *** Static Member Functions ***
 
 * - Utility - *

 void setNumFluxes(int)
    Inline function sets the number of fluxes.

 static int getNumFluxes()
    Inline function queries the number of fluxes.

 static void setNumGroups(int)
    Inline function sets the number of neutron groups.

 static int getNumGroups() 
    Inline function queries the number of neutron groups.
 
 *** Member Functions ***

 * - Constructors & Destructors - *

 VolFlux()
    Default constructor creates storage for 'flux' if 'nGroups'>0,
    otherwise sets 'flux' to NULL.  Always sets 'next' to NULL.

 VolFlux(const VolFlux&)
    Copy constructor copies 'flux' on element-by-element basis.  Sets
    next to NULL.

 VolFlux(ifstream &, double )
    This constructor reads the flux values from a file attached to the
    first argument stream reference and scales them by the second
    argument.

 ~VolFlux()
    Inline destructor delets storage for 'flux' and destroys list of
    VolFlux objects by deleting 'next'.

 VolFlux& operator=(const VolFlux&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the object will continue to be part of the
    same list unless explicitly changed.


 * - Input - *

 VolFlux* read(ifstream &, double )
    This function extends the list of flux info by passing the
    arguments to the VolFlux(ifstream&,double) constructor.  The newly
    created object is pointed to by the 'next' of the object through
    which the function is called and a pointer to the newly created
    object is returned.
 
 * - Solution - *

 double updateReference(VolFlux*)
    This function compares the flux of the object through which it is
    called with the object pointed to by the argument, and sets the
    group-wise maximum flux.

 double fold(double*)
    This function takes a rate vector pointed to by the argument and
    folds it with this flux, returning the scalar reaction rate.

 * - Utility - *

 VolFlux* advance()
    Inline function provides access to the 'next' object in the list.

 */

#ifndef _VOLFLUX_H
#define _VOLFLUX_H


class VolFlux
{
protected:
  static int nFluxes, nGroups;

  double *flux;

  VolFlux *next;

public:
  /* Utility */
  static void setNumFluxes(int numFlx)
    { nFluxes = numFlx; };
  static int getNumFluxes() 
    { return nFluxes; };
  static void setNumGroups(int numGrps)
    { nGroups = numGrps; };
  static int getNumGroups() 
    { return nGroups; };

  /* Service */
  VolFlux();
  VolFlux(const VolFlux&);
  VolFlux(ifstream &, double );
  ~VolFlux()
    { delete flux; delete next; };

  VolFlux& operator=(const VolFlux&);

  /* Input */
  VolFlux* read(ifstream &, double );
  
  /* Solution */
  void updateReference(VolFlux*);
  double fold(double*);

  /* Utility */
  VolFlux* advance() {return next;};

};


#endif
