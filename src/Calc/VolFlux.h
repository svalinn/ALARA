/* $Id: VolFlux.h,v 1.12 2002-08-23 20:45:31 fateneja Exp $ */
#include "alara.h"

#ifndef _VOLFLUX_H
#define _VOLFLUX_H

#define REFFLUX_MAX     0
#define REFFLUX_VOL_AVG 1

#include "RateCache.h"

/** \brief This class stores the information about the neutron fluxes in 
 *         each interval.  
 *         
 *  Each Volume object will have a linked list of these VolFlux
 *  objects, with each element in the list representing a different flux
 *  specification.
 */

class VolFlux
{
protected:
  static int 
    /// The number of user-defined Flux specifications.
    nFluxes, 

    /// The number of neutron groups being used in this problem.
    nGroups, 
    
    /// An indicator of which type of reference flux should be used for
    /// truncation calculations.
    refflux_type;

  double 
    /// An array of scalar flux values.
    *flux;

  RateCache 
    /// A data cache to prevent refolding the cross-sections with the
    /// fluxes too often.  (see RateCache)
    cache;

  VolFlux 
    /// A pointer to the next VolFlux object in this list.
    *next;

public:
  /// Inline function sets the number of fluxes.
  static void setNumFluxes(int numFlx)
    { nFluxes = numFlx; };

  /// Inline function queries the number of fluxes.
  static int getNumFluxes() 
    { return nFluxes; };
  
  /// Inline function sets the number of neutron groups.
  static void setNumGroups(int numGrps)
    { nGroups = numGrps; };

  /// Inline function queries the number of neutron groups.
  static int getNumGroups() 
    { return nGroups; };
  
  /// Inline function sets the type of reference flux to use.
  static void setRefFluxType(char refflux_type_code)
    { 
      switch (refflux_type_code) {
      case 'v':
	refflux_type = REFFLUX_VOL_AVG;
	break;
      case 'm':
      default:
	refflux_type = REFFLUX_MAX;
	break;
      }
    };

  /// Inline function queries the reference flux type.
  static int getRefFluxType()
    { return refflux_type; } ;

  /// Default Constructor
  /** This constructor creates storage for 'flux' if 'nGroups'>0,
    otherwise sets 'flux' to NULL.  Always sets 'next' to NULL. */
  VolFlux();

  /// Copy constructor 
  /** This constructor copies 'flux' on element-by-element basis.  Sets
      next to NULL. */
  VolFlux(const VolFlux&);

  /// This constructor reads the flux data from an array.
  /** It takes the values in the array and stores them in VolFlux::Flux */  
  VolFlux(double*, double);
  
  /// This constructor reads the flux values from a file attached to the
  /// first argument stream reference and scales them by the second
  /// argument.
  VolFlux(ifstream &, double );
   
  
  /** Inline destructor deletes storage for 'flux' and destroys list of
       VolFlux objects by deleting 'next'. */
  ~VolFlux()
    { delete flux; delete next; };

  /// Overloaded assignment operator
  /** The correct implementation of this operator must ensure that
      previously allocated space is returned to the free store before
      allocating new space into which to copy the object. Note that
      'next' is NOT copied, the object will continue to be part of the
      same list unless explicitly changed. */
  VolFlux& operator=(const VolFlux&);

  /// This function extends the list of flux info by passing the
  /// arguments to the VolFlux(ifstream&,double) constructor.  
  /** The newly created object is pointed to by the 'next' of the 
      object through which the function is called and a pointer to the
      newly created object is returned. */
  VolFlux* read(ifstream &, double );

  VolFlux* copyData(double *, double);

  /// This function compares the flux of the object through which it is
  /// called with the object pointed to by the argument, and sets the
  /// the reference flux. 
  /** There are currently two options:
        1) group-wise maximum flux, or
        2) group-wise volume weighted average flux */
  void updateReference(VolFlux*,double);

  /// This function scales the fluxes but multiplying by the first 
  /// argument.
  void scale(double);
  
  /// This function takes a rate vector pointed to by the first argument
  /// and folds it with this flux, returning the scalar reaction rate.
  /** The second argument points to the Node object associated with this
      rate vector and will be used to determine the indexing
      information for the RateCache (see RateCache). */
  double fold(double*,Node*);

  /// Inline function provides access to the 'next' object in the list.
  VolFlux* advance() {return next;};

};


#endif
