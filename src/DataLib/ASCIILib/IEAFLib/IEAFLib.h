/* $Id: IEAFLib.h,v 1.2 2001-09-10 23:02:44 wilsonp Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class provides an interface to non-binary cross-section libraries 
following the GENDF format and decay/gamma libraries following
the EAF Format (basically and ENDF/B-6 format).  This class is derived
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
-------------------------------------------------------------------

 *** Class Members ***

 inTrans : ifstream
    This is the input file stream for the transmutation library.

 inDecay : ifstream
    This is the input file stream for the decay library.

 *** Protected Member Functions ***

 * - ASCIILib Interface - *

 void getTransInfo()
    This function provides the necessary internal function to read the
    header information of an GENDF formatted transmutation data file.

 void getDecayInfo()
    This function provides the necessary internal function to read the
    header information of an EAF formatted decay data file.

 int getTransData()
    This function provides the necessary internal function to read a
    single entry (the next entry) from an GENDF formatted transmutation
    data file.

 int getDecayData();
    This function provides the necessary internal function to read a
    single entry (the next entry) from an EAF formatted decay data
    file.
 
 *** Member Functions ***

 * - Cosntructors & Destructors - *

 IEAFLib(char*,char*)
    This default constructor invokes the default constructor of the
    ASCIILib class, passing the DATALIB_IEAF definition as its
    argument.  If either of the arguments are not given to this
    constructor, nothing more is done.  Otherwise, the two non-binary
    libraries are openned with file streams and a binary library is
    created using the inherited ASCIILib::makeBinLib().
 
 */

#ifndef IEAFLIB_H
#define IEAFLIB_H

#define DATALIB_IEAF 6

#define MAXLINELENGTH 85
#define MAXIEAFRXNS 350
#define MAXIEAFDCYMODES 15

#include "../ASCIILib.h"

class IEAFLib : public ASCIILib
{
protected:
  
  ifstream inTrans, inDecay;

  /* Read Non-Binary Data */
  /* Utility */
  void extract(char*,float*);
  /* Internal */
  int delKza(float);
  void skipDiscreteGammas(char*,int);
  void skipContGammas(char*);
  void readDiscreteGammas(int, int, float, char*);
  void readContGammas(int, float, char*);
  int getGammaData();

  /* Interface from ASCIILib */
  void getTransInfo();
  void getDecayInfo();
  int getTransData();
  int getDecayData();

public:
  /* Service */
  IEAFLib(char*, char*, char*);
  ~IEAFLib();

};

#endif
