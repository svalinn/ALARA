/* $Id: Flux.h,v 1.12 2003-01-13 04:34:55 fateneja Exp $ */
#include "alara.h"

#ifndef _FLUX_H
#define _FLUX_H

/* flux formats */
#define FLUX_HEAD 0
#define FLUX_D 1
#define FLUX_R 2

/* results of searching for flux */
#define FLUX_NOT_FOUND -1
#define FLUX_BAD_FNAME -2

/** \brief This class is invoked as a linked list and describes the 
 *         flux information of the problem.
 *   
 *  The single object of class Input has a list of Fluxes.  The first
 *  element in each list has type FLUX_HEAD (defined through the '
 *  format' member), and contains no problem data.
 */

class Flux
{
protected:
  /// This indicates the format of this flux and is based on the
  /// definitions given below.
  int format;
    
  /// The number of full interval entries to skip before the first one
  /// to be read.
  /** This makes it easy to either skip entries not modelled in the
      geometry, use the same file for a number of flux
      descriptions - subsequent descriptions need to skip the initial
      ones.  Again, note that this is a count of how many *full interval
      entries* - one entry has nGroup data values. */
  int skip;

  /// A scalar normalization for this flux description.
  /** This allows re-normalization of flux spectra either for physical
      or modelling reasons. */
  double scale;

  /// The descriptive name of this flux description.
  /** This will be used primarily to cross-reference this flux 
      description with the schedules. */
  char *fluxName;
    
  /// The filename where these flux spectra should be read.
  char *fileName;

  /// The pointer to the next flux description in the list.
  Flux* next;

public:
  /// Default constructor
  Flux(int inFormat=FLUX_HEAD, char* flxName=NULL, 
       char* fName=NULL, double Scale=0, 
       int inSkip=0);

  /// Copy constructor
  Flux(const Flux&);

  /// Inline destructor 
  /** Deletes storage for the flux identifier and the flux file name.
      The whole list is destroyed by deleting the next pointer. */
  ~Flux()
    { delete fileName; delete fluxName; delete next; };

  /// Overloaded assignment operator
  Flux& operator=(const Flux&);

  /// Function to read the flux description from the input file
  /// connected to the stream given in the first argument.
  Flux* getFlux(istream&);

  /// Setup the problem to read the actual flux data into containers in each
  /// interval.
  void xRef(Volume*);
  
  /// Find a specific flux description based on the flux identifier give
  /// as the argument.
  int find(char*);

  /// Check that the filename specified in the description is useful by
  /// openning and closing again.
  int checkFname();

  /// Count the number of flux descriptions in the list.
  int count();
};



#endif
