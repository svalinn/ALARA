/* $Id: DataLib.h,v 1.8 2006-01-16 19:11:51 phruksar Exp $ */
#include "alara.h"

/** \brief This class is the base class for the handling of all nuclear
           data libraries.  
 
    Various classes are derived from this class which may be instantiated
    directly or serve as base classes for another class.  All data libraries
    have some common members, which are declared as members of this base 
    class, but is serves primarily to define the interface to the data 
    libraries.

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
    DATALIB_IEAF     6     ieaf     A data library with cross-section
    libraries following the GENDF format and 
    decay/gamma libraries following the 
    formatting definition of the EAF library 
    (roughly ENDF/B-6) 
    DATALIB_FEIND   7      feind    A FEIND library
    -------------------------------------------------------------------
    */
    
#ifndef _DATALIB_H
#define _DATALIB_H

#define DATALIB_NULL 0

/* library conversion codes */
#define EAF2ALARA 301
#define IEAF2ALARA 601
#define ALARA2ADJ 104

#include "Chains/NuclearData.h"
#include "Output/GammaSrc.h"

extern const char *libTypes;
extern const int libTypeLength;
extern const char *libTypeStr[];
extern const char *libTypeSuffix[];

class DataLib
{
protected:
  
  /// This indicates the type of library based on the above catalogue of
  /// types.
  int type;

  /// This indicates the resolution of the bottom level entry in a data
  /// library.
  /** For example, if each cross-section described in a transmutation 
      library has M neutron group, then 'nGroups' is M. */
  int nGroups;

  /// This indicates how many top-level entries exist in a data library.
  /** For example, a transmutation cross-section library with data for N
      isotopes has N top-level entries. */
  int nParents;

  /// This function converts a text string argument into an integer
  /// library type as described in the catalogue above.
  static int convertLibType(char*);

public:
  /// Although not formally a constructor, this function acts as one.
  static DataLib* newLib(char*,istream&);

  // NEED COMMENT
  static void convertLib(char*, int,istream&);

  // NEED COMMENT
  static void convertLib(istream&);

  /// When called with no arguments, this is the inline default constructor
  /** Creates a new object with type DATALIB_NULL.  Otherwise, the type
      is specified by the argument.  'nGroups' and 'nParents' are both
      set to 0 in all cases. */
  DataLib(int setType=DATALIB_NULL) 
    { type = setType; nGroups = 0; nParents = 0;};
  virtual ~DataLib() {};

  /// This inline function provides read access to the 'nGroups' member.
  int getNumGroups()
    { return nGroups; };

  /// This defines the interface function readData(...) to the hierarchy
  /// of DataLib classes.
  virtual void readData(int,NuclearData*);

  // NEED COMMENT
  virtual void readGammaData(int, GammaSrc*);

};


#endif
