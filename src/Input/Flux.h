/* $Id: Flux.h,v 1.8 2000-01-17 16:57:38 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list and describes the flux
information of the problem.  The single object of class Input has a
list of Fluxes.  The first element in each list has type FLUX_HEAD
(defined through the 'format' member), and contains no problem data.

 *** Class Members ***

 format : int
    This indicates the format of this flux and is based on the
    definitions given below.

 skip : int 
    The number of full interval entries to skip before the first one
    to be read.  This makes it easy to either skip entries not
    modelled in the geometry, use the same file for a number of flux
    descriptions - subsequent descriptions need to skip the initial
    ones.  Again, note that this is a count of how many *full interval
    entries* - one entry has nGroup data values.

 scale : double
    A scalar normalization for this flux description.  This allows
    re-normalization of flux spectra either for physical or modelling
    reasons.

 fluxName : char*
    The descriptive name of this flux description.  This will be used
    primarily to cross-reference this flux description with the
    schedules.

 fileName : char*
    The filename where these flux spectra should be read.

 next : Flux*
    The pointer to the next flux description in the list.

 *** Member Functions ***

 * - Constructors & Destructors - *

 Flux(int,char*,char*,double,int)
    Default constructor creates a blank list head when no arguments
    are given.  Otherwise, it sets the format, flux identifier, flux
    file name, scaling factor, and skip value, with arguments given in
    that order.

 Flux(const Flux&)
    Copy constructor initializes 'scale', 'skip' and 'format' and then
    creates and fills storage for 'fluxName' and 'fileName'.

 ~Flux()

    Inline destructor deletes storage for the flux identifier and the
    flux file name.  The whole list is destroyed by deleting the next
    pointer.

 Flux& operator=(const Flux&)
    This assignmnet operator behaves similarly to the copy
    constructor. The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.  It does NOT
    copy 'next'.


 * - Input - *

 Flux* getFlux(istream&)
    Function to read the flux description from the input file
    connected to the stream given in the first argument.  It returns a
    pointer to the new object of class Flux which has just been
    created.  It does NOT read the actual flux information from the
    file.

 * - Preproc - *

 void xRef(Volume*)
    Setup the problem to read the actual flux data into containers in
    each interval.  The function expects a pointer to an object of
    class Volume which should be the head of the global interval list.
 
 * - Utility - *

 int find(char*) 
    Find a specific flux description based on the flux identifier give
    as the argument.  ** Returns the 0-based ordinal number of the
    flux in the list. ** Special values (< 0) are returned for special
    cases, such as bad file names or unfound flux descriptions.

 int checkFname()
    Check that the filename specified in the description is useful by
    openning and closing again.

 int count()
    Count the number of flux descriptions in the list.

 */

#ifndef _FLUX_H
#define _FLUX_H

/* flux formats */
#define FLUX_HEAD 0
#define FLUX_D 1

/* results of searching for flux */
#define FLUX_NOT_FOUND -1
#define FLUX_BAD_FNAME -2

class Flux
{
protected:
  int format, skip;
  double scale;
  char *fluxName, *fileName;

  Flux* next;

public:
  /* service */
  Flux(int inFormat=FLUX_HEAD, char* flxName=NULL, 
       char* fName=NULL, double Scale=0, 
       int inSkip=0);
  Flux(const Flux&);
  ~Flux()
    { delete fileName; delete fluxName; delete next; };

  Flux& operator=(const Flux&);

  /* Input */
  Flux* getFlux(istream&);

  /* Preproc */
  void xRef(Volume*);
  
  /* Utility */
  int find(char*);
  int checkFname();
  int count();
};



#endif
