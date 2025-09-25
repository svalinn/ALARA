/* $Id: EAFLib.h,v 1.6 2001-07-10 20:51:30 wilsonp Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class provides an interface to non-binary libraries following the
EAF Format (basically and ENDF/B-6 format).  This class is derived
directly and publicly from class ASCIILib (which is derived from
DataLib).  

*** Supported Data Library Formats ***

This catalogue of data library types should be included in every new
module developed to support a new data library format.

                       Input
Identifier     Value   String   Description
-------------------------------------------------------------------
DATALIB_NULL     0     null     A basic DataLib object
                                (should rarely be used in final object)
DATALIB_ALARA    1     alara    The default ALARA v1 binary format.
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
DATALIB_FEIND    7     feind    A FEIND library
DATALIB_ALARAJOY 8     ajoy     A hybrid data library following the formatting
                                definition of the FENDL 3.2b (TENDL 2017)
                                library (TENDL and PENDF format, converted
                                to GENDF format by an NJOY wrapped Python 
                                preprocessor) for transmutation and the EAF
                                library for decay.     
-------------------------------------------------------------------

 *** Class Members ***

 inTrans : ifstream
    This is the input file stream for the transmutation library.

 inDecay : ifstream
    This is the input file stream for the decay library.

 *** Protected Member Functions ***

 * - Read Non-Binary Data - *

 void readReaction(ifstream&, int&, int)
   This function reads and parses the information for a single
   transmutation reaction.

 * - ASCIILib Interface - *

 void getTransInfo()
    This function provides the necessary internal function to read the
    header information of an EAF formatted transmutation data file.

 void getDecayInfo()
    This function provides the necessary internal function to read the
    header information of an EAF formatted decay data file.

 int getTransData()
    This function provides the necessary internal function to read a
    single entry (the next entry) from an EAF formatted transmutation
    data file.

 int getDecayData();
    This function provides the necessary internal function to read a
    single entry (the next entry) from an EAF formatted decay data
    file.
 
 *** Member Functions ***

 * - Cosntructors & Destructors - *

 EAFLib(char*,char*)
    This default constructor invokes the default constructor of the
    ASCIILib class, passing the DATALIB_EAF definition as its
    argument.  If either of the arguments are not given to this
    constructor, nothing more is done.  Otherwise, the two non-binary
    libraries are opened with file streams and a binary library is
    created using the inherited ASCIILib::makeBinLib().
 
 */

#ifndef EAFLIB_H
#define EAFLIB_H

#define DATALIB_EAF 3

#define MAXLINELENGTH 85
#define MAXEAFRXNS 200
#define MAXEAFDCYMODES 100

#include "ASCIILib.h"

class EAFLib : public ASCIILib
{
protected:
  
  ifstream inTrans, inDecay;

  /* Read Non-Binary Data */
  /* Utility */
  void extract(char*,float*);
  int delKza(float);
  /* Internal */
  void readReaction(int&, int);
  void skipDiscreteGammas(char*,int);
  void skipContGammas(char*);
  void readDiscreteGammas(int, int, float, char*);
  void readContGammas(int, float, char*);

  /* Interface from ASCIILib */
  void getTransInfo();
  void getDecayInfo();
  int getTransData();
  int getDecayData();
  int getGammaData();

public:

  /* Service */
  EAFLib(const char*, const char*, const char*);
  // Decay-only constructor for ALARAJOYLib (no makeBinLib calling)
  EAFLib(const char* decayFname, bool decayOnly);
  ~EAFLib();

};

#endif
