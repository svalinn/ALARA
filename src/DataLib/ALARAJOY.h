#include "alara.h"
#include "EAFLib.h"

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
DATALIB_FEIND    7     feind    A FEIND library
DATALIB_ALARAJOY 8     ajoy     A hybrid data library following the formatting
                                definition of the FENDL 3.2b (TENDL 2017)
                                library (TENDL and PENDF format, converted
                                to GENDF format by an NJOY wrapped Python 
                                preprocessor) for transmutation and the EAF
                                library for decay.    
-------------------------------------------------------------------
*/

#ifndef ALARAJOY_H
#define ALARAJOY_H

#define DATALIB_ALARAJOY 8

#define MAXALARAJOYRXNS 350
#define MAXALARAJOYEMITTEDSTR 6

struct DSVRow {
    int parentKZA;
    int daughterKZA;
    std::string emittedParticles;
    int nonZeroGroups;
    std::vector<float> crossSections;
};

class ALARAJOYLib : public EAFLib
{
    protected:
        ifstream inTrans;
    
    public:
        /* Constructor, & Destructor
            Note: tranFname refers to a CSV of preprocessed FENDL3 data from
            an external Python script that converts TENDL/PENDF pairs to GENDF
            data using the NJOY GROUPR module and writes it out to a CSV. If
            CSV has not yet been processed, run 
            ./ALARAJOY_wrapper/preprocess_fendl3.py, following
            ./ALARAJOY_wrapper/README.md instructions for usage prior to running
            ALARA convert_lib with ALARAJOY.
            Additionally, decay data is not accessed through FENDL3, but
            rather in conjunction with EAF data, accessing EAFLib decay
            methods in conjunction with ALARAJOY-specific transmutation*/
        ALARAJOYLIB(const char* transFname, const char* decayFname, const char* alaraFname);
        ~ALARAJOYLIB();

    private:

        // Variables to hold pre-loaded space-delimter DSV transmutation data
        static std::vector<DSVRow> dsvData;
        size_t currentRowIndex;
        int currentParent;

        void loadDSVData();
        
};

#endif