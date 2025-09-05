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
                                (roughly ENDF/B-6).
DATALIB_FENDL3   7     fendl    A data library following the formatting
                                definition of the FENDL 3.2b (TENDL 2017)
                                library (TENDL and PENDF format, converted
                                by preprocessor to GENDF format).   
-------------------------------------------------------------------

*/

#ifndef FENDL3_H
#define FENDL3_H

#define DATALIB_FENDL3 7

#define MAXFENDL3RXNS 350

#include "ASCIILib.h"

class FENDL3LIB : public ASCIILib
{
    protected:
        
        ifstream inTrans;

        /* Interface from ASCIILib */
        void getTransInfo();
        void getDecayInfo();
        int getTransData();
        int getDecayData();

        /* Internal helpers */
        void loadCSVData();
    
    public:
        /* Constructor, & Destructor */
        FENDL3LIB(const char* transFname, const char* decayFname, const char* alaraFname);
        ~FENDL3LIB();
};

#endif