/* $Id: ASCIILib.h,v 1.5 2001-07-10 20:51:04 wilsonp Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class provides a base class to data libraries which are
non-binary.  This class is derived directly and publicly from class
DataLib.  Other classes specific to a particular non-binary library
are derived from this class.  This class is intrinsically dependent on
the definition of class ALARALib because it creates a library with the
ALARA v1 binary format.

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
DATALIB_FENDL3   8     fendl    A data library following the formatting
                                definition of the FENDL 3.2b (TENDL 2017)
                                library (TENDL and PENDF format, converted
                                by preprocessor to GNENDF format).                                   
-------------------------------------------------------------------

 *** Class Members ***

 transTitle : char[]
    This is the title of the transmutation library.

 decayTitle : char[]
    This is the title of the decay library.

 groupDesc : char[]
    This is a text description which may be given to the group
    structure and/or flux weighting.

 grpBnds : float*
    An array containing the boundary energies of the flux groups used
    to create the group cross-sections.

 grpWeights : float*
    An array containing the intergral flux weights for each flux
    group used to create the group cross-sections.

 * - Transmutation Data - *

 nTRxns : int 
    The number of transmutation reactions for a given isotope.

 transKza : int*
    The KZA numbers of the transmutation daughters.

 xSection : float**
    A list of rate vectors for the transmutation reactions.

 emitted : char**
    A list of character strings indicating the emitted
    particles/reaction types for the transmutation reactions.

 * - Decay Data - *

 nDRxns : int
    The number of decay reactions for this isotope.

 nIons : int
    The number of decay products which are enmitted light ions rather
    than residual nuclei.

 decayKza : int*
    A list of KZA numbers for the daughter products of the decay
    reaction.

 thalf : float
    The half life of the isotope.

 bRatio : float*
    The list of branching ratios for each of the decay reactions.

 E : float[3]
    The average decay heat of beta, gamma and alpha channels.

 * - Merged Data - *

 kza : int
    The KZA of the parent isotope.

 nRxns : int
    The total number of reaction paths, counted by the number of
    unique reaction products.

 daugKza : int*
    The list of unique reaction product KZA numbers.

 mThalf : float    
    The half life of the parent isotope.

 mE : float[3]
    The decay heat of the beta, gamma and alpha channels.

 mXsection : float**
    The merged list of pseudo-cross-section vectors for all the
    reaction paths.  These are 'pseudo' because an extra group is used
    to store the decay rate of a particular reaction path.

 mEmitted : char**
    The merged list of emitted particles and reaction types for each
    of the reaction paths.  Where reactions have been merged (because
    they have the same reaction product), the emitted string has a
    comma separated list of reaction type information.

 * - File Pointers - *

 tmpIdx : fstream
    This is the temporary text index which is created during the
    writing of a binary library.  This is then copied onto the end of
    the binary library in binary format.

 binLib : ALARALib*
    This is the pointer to the binary file created from the ASCII
    library.

 *** Protected Member Functions ***

 * - Merge Data - *

 void trans2merge() 

    This function creates a set of "merged" data out of only the set
    of "transmutation" data.  It initializes the decay-relevant values
    to 0 and copies the transmutation-relevant values to the
    appropriate storage.  The transmutation paths are reduced by
    merging all non-emission(*) pathways which have the same reaction
    product and by removing all pathways with zero cross-section.

 void decay2merge()

    This function creates a set of "merged" data out of only the set
    of "decay" data.  It initializes the transmutation-relevant values
    to 0 and copies the decay-relevant values to the appropriate
    storage.  The decay paths are reduced by merging all
    non-emission(*) pathways which have the same reaction product and
    by removing all pathways with zero cross-section.

 void merge()
    This function creates a set of "merged" data from both the
    "transmutation" set and "decay" set of data, copying the relevant
    information from both sets.  The reaction paths are reduced by
    merging all non-emission(*) pathways which have the same reaction
    product and by removing all pathways with zero cross-section.

    (*) non-emission pathways are all those pathways leading a
    residual nucleus daughter product.  A single parent isotope may
    have both non-emission and emission pathways to the same product,
    but those pathways will not be merged.

 * - Write Binary Data - *

 void writeData(long&)
    This function writes a merged data set to the binary file and adds
    an entry to the index.

 void appendIdx()
    After writing all the top-level entries, including indexing, the
    index is then appended to the end of the binary file in binary
    format.

 *** Protected Virtual Member Functions ***

 * - Read Non-Binary Data - *

 virtual void getTransInfo()
    This function reads the header information on a non-binary
    transmutation data file.

 virtual void getDecayInfo()
    This function reads the header information on a non-binary
    decay data file.

 virtual int getTransData();
    This function reads the transmutation data for a single parent
    isotope.

 virtual int getDecayData();
    This function reads the decay data for a single parent isotope.

 *** Member Functions ***

 * - Cosntructors & Destructors - *

 ASCIILib(int)
    This default constructor invokes the DataLib default constructor
    passing the argument, which in turn defaults to DATALIB_ASCII.
    The 'binLib' member is then set to NULL.

 ~ASCIILib()
    This destructor closes both the 'binLib' and 'tmpIdx' file
    pointers/references.
 
 * - Binary Libaray Management - *

 void makeBinLib()
    This function is in charge of creating a library with the ALARA v1
    binary format out of the non-binary library being accessed.  It
    cycles through both the transmutation and decay libraries getting
    the various data sets, merging them, and writing them to the
    binary library.

 * - Utility - *

 float sum(float*)
    This function sums the elements of a rate vector pointed to by the
    first argument.

 * - Virtual - *

 void readData(int,NuclearData*)
    This is the required instance of the readData interface function
    for this class.  It is not a valid to call this function since
    ASCIILib should only be a base class for a more specific library
    format.

 */

#ifndef ASCIILIB_H
#define ASCIILIB_H

#include "DataLib/DataLib.h"

#define DATALIB_ASCII 2

class ASCIILib : public DataLib
{
protected:
  char transTitle[81],decayTitle[81],groupDesc[81];
  float *grpBnds,*grpWeights;

  /* for each isotope */
  /*  -- trans -- */
  int nTRxns, *transKza;
  float **xSection;
  char **emitted;
  /*  -- decay -- */
  int nDRxns, nIons, *decayKza;
  float thalf, *bRatio, E[3];
  /*  -- merged -- */
  int kza, nRxns, *daugKza;
  float mThalf, mE[3], **mXsection;
  char **mEmitted;
  /*  -- gamma -- */
  int numSpec;
  int *numDisc, *nIntReg, *nPnts;
  int **intRegB, **intRegT;
  float **discGammaE, **discGammaI, **contX, **contY;

  ALARALib *binLib, *gammaLib;

  /* Merge Data */
  void trans2merge();
  void decay2merge();
  void merge();

  /* Virtual */
  /* Read Non-Binary Data */
  virtual void getTransInfo();
  virtual void getDecayInfo();
  virtual int getTransData();
  virtual int getDecayData();

public:
  /* Service */
  ASCIILib(int setType=DATALIB_ASCII);
  ~ASCIILib();
  
  /* Binary Libaray Management */
  void makeBinLib(const char*);

  /* Utility */
  float sum(float*);

  /* Virtual */
  virtual void readData(int,NuclearData*);
  
};

#endif
