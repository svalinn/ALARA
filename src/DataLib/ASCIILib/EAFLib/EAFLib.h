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
    libraries are openned with file streams and a binary library is
    created using the inherited ASCIILib::makeBinLib().
 
 */

#ifndef _EAFLIB_H
#define _EAFLIB_H

#define DATALIB_EAF 3

#define MAXLINELENGTH 85
#define MAXEAFRXNS 50
#define MAXEAFDCYMODES 15

#include "../ASCIILib.h"

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
  int getGammaData();

  /* Interface from ASCIILib */
  void getTransInfo();
  void getDecayInfo();
  int getTransData();
  int getDecayData();

public:
  /* Service */
  EAFLib(char*, char*, char*);
  ~EAFLib();

};

#endif
