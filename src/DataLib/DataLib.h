/* $Id: DataLib.h,v 1.5 2000-07-07 02:15:51 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is the base class for the handling of all nuclear data
libraries.  Various classes are derived from this class which may be
instantiated directly or serve as base classes for another class.  All
data libraries have some common members, which are declared as members
of this base class, but is serves primarily to define the interface to
the data libraries.  

*** Supported Data Library Formats ***

This catalogue of data library types should be included in every new
module developed to support a new data library format.

                       Input
Identifier     Value   String   Description
-------------------------------------------------------------------
DATALIB_NULL     0     null     A basic DataLib object
                                (should rarely be used in final object)
DATALIB_ALARA    1     alara    The default ALARA v1 binary mreged format.
DATALIB_ASCII    2     ascii    A basic ASCII DataLib object
                                (should rarely be used in final object)
DATALIB_EAF      3     eaf      A data library following the formatting
                                definition of the EAF library (roughly
                                ENDF/B-6) 
DATALIB_ADJOINT  4     adj      An alara binary library in reversed format
                                for reverse calculations.
DATALIB_GAMMA    5     gamma    An alara binary library containing gamma
                                source information.
-------------------------------------------------------------------

 *** Class Members ***

 type : int 
    This indicates the type of library based on the above catalogue of
    types.

 nParents : int
    This indicates how many top-level entries exist in a data library.
    For example, a transmutation cross-section library with data for N
    isotopes has N top-level entries.

 nGroups : int
    This indicates the resolution of the bottom level entry in a data
    library.  For example, if each cross-section described in a
    transmutation library has M neutron group, then 'nGroups' is M.

 *** Static Protected Member Functions ***

 * - Utility - *
 int convertLibType(char*)
    This function converts a text string argument into an integer
    library type as described in the catalogue above.

 *** Static Member Functions ***

 * - Constructors & Destructors - *

 DataLib* newLib(char*,istream&)
    Although not formally a constructor, this function acts as one.
    It takes the text string argument and converts it to an integer
    type, and then based on that type creates a new object of the
    correct derived type reading further information, as required,
    from the inpout file attached to the passed stream reference.  A
    pointer to the newly created object.

 *** Member Functions ***
 
 * - Constructors & Destructors - *

 DataLib(int) 
    When called with no arguments, this inline default constructor
    creates a new object with type DATALIB_NULL.  Otherwise, the type
    is specified by the argument.  'nGroups' and 'nParents' are both
    set to 0 in all cases.
 
 * - Utility - *    

 int getNumGroups()
    This inline function provides read access to the 'nGroups' member.

 *** Virtual Member Functions ***

 void readData(int,NuclearData*)
    This defines the interface function readData(...) to the hierarchy
    of DataLib classes.  This must be redefined in the derived
    classes.  The arguments are intended to be the KZA number of an
    isotope in question and a pointer to the NuclearData object which
    will store the data being read.

*/


#ifndef _DATALIB_H
#define _DATALIB_H

#define DATALIB_NULL 0

/* library conversion codes */
#define EAF2ALARA 301
#define ALARA2ADJ 104

#include "Chains/NuclearData.h"
#include "Output/GammaSrc.h"

extern const char *libTypes;
extern const int libTypeLength;
extern const char *libTypeStr[];
extern const char *libTypeSuffix[];

/* This is a base class and should never be instantiated */

class DataLib
{
protected:
  int type;
  int nGroups, nParents;

  /* Utility */
  static int convertLibType(char*);

public:
  /* Service */
  static DataLib* newLib(char*,istream&);
  static void convertLib(char*, int,istream&);
  static void convertLib(istream&);

  DataLib(int setType=DATALIB_NULL) 
    { type = setType; nGroups = 0; nParents = 0;};
  virtual ~DataLib() {};

  /* Utility */
  int getNumGroups()
    { return nGroups; };

  /* Virtual */
  virtual void readData(int,NuclearData*);
  virtual void readGammaData(int, GammaSrc*);

};


#endif
