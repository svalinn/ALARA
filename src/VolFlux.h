/* $Id: VolFlux.h,v 1.15 2003-01-13 04:34:28 fateneja Exp $ */
#include "alara.h"

#ifndef VOLFLUX_H
#define VOLFLUX_H

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
  /// The number of user-defined Flux specifications.
  static int nFluxes;

  /// The number of neutron groups being used in this problem.
  static int nGroups; 
    
  /// An indicator of which type of reference flux should be used for
  /// truncation calculations.
  static int refflux_type;
    
  /// Number of charged particles in problem
  static int nCP;
    
  /// Number of charged particle energy groups
  static int nCPEG;

  /// Neutron flux storage
  double *nflux;
    
  /// Charged particle flux storage
  double **CPflux;

  /// Storage for Charged particle flux
  double *CPfluxStorage;

  /// A data cache to prevent refolding the cross-sections with the
  /// fluxes too often.  (see RateCache)
  RateCache cache;

  /// A pointer to the next VolFlux object in this list.
  VolFlux *next;

public:
  /// Inline function to set number of Charged Particles
  static void setNumCP(int numCP) { nCP = numCP; };

  /// Inline function to get number of Charged Particles
  static int getNumCP() { return nCP; };

  /// Inline function to set number of Charged Particle Energy Groups
  static void setNumCPEG(int numCPEG) { nCPEG = numCPEG; };

  /// Inline function to get number of Charged Particle Energy Groups
  static int getNumCPEG() { return nCPEG; };

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
  VolFlux();

  /// Copy constructor 
  VolFlux(const VolFlux&);

  /// This constructor reads the flux data from an array.
  VolFlux(double*, double);
  
  /// This constructor reads the flux values from a file attached to the
  /// first argument stream reference and scales them by the second
  /// argument.
  VolFlux(ifstream &, double );
   

  /// Inline destructor
  /** Deletes storage for 'flux' and destroys list of VolFlux objects by
      deleting 'next'. Also deletes CPflux*/
  ~VolFlux()
    { delete[] nflux; delete next; delete[] CPflux;};

  /// Overloaded assignment operator
  VolFlux& operator=(const VolFlux&);

  /// This function extends the list of flux info by passing the
  /// arguments to the VolFlux(ifstream&,double) constructor.  
  VolFlux* read(ifstream &, double );

  /// NEED COMMENT
  VolFlux* copyData(double *, double);

  /// This function compares the flux of the object through which it is
  /// called with the object pointed to by the argument, and sets the
  /// the reference flux. 
  void updateReference(VolFlux*,double);

  /// This function scales the fluxes but multiplying by the first 
  /// argument.
  void scale(double);
  
  /// This function takes a rate vector pointed to by the first argument
  /// and folds it with this flux, returning the scalar reaction rate.
  double fold(double*,Node*);

  /// Inline function provides access to the 'next' object in the list.
  VolFlux* advance() {return next;};

  /// Access function for the neutron flux
  double *getnflux() {return nflux;};

  /// Access function for the Charged Particle flux
  double **getCPflux() {return CPflux;};

  /// Function to see the Charged Particle flux
  void setCPflux(int CP, int CPEG, double value)
    {CPflux[CP][CPEG] = value;}
};


#endif
