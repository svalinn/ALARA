/* $Id: ALARALib.h,v 1.7 2001-07-10 20:51:16 wilsonp Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class provides access to data libraries which follow the ALARA v1
binary merged library format.  This class is derived directly and
publicly from class DataLib.

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

 *** Locally Defined Classes ***

 LibIdx
 
    Indexing a data library with ALARA v1 binary format is done with
    class LibIdx.  This class stores an array of KZA numbers and
    binary file offsets, then provides a mechanism to search for a
    given KZA entry.  The ALARA v1 binary format has an index at the
    end of the file which is read to create this index.

    *** Class Members ***

    nParents : int
       This indicates the size of the index, that is, the number of
       entries in the KZA list and corresponding offset list.

    kza : int*
       This is the list of KZA values for each of the top-level
       entries in the library.  They are sorted by base ZA number, the
       order they appear in the library.

    offset : long*
       This is the list of file offsets which correspond to the list
       of KZA values.

    *** Member Functions ***

    * - Constructors & Destructors - *

    LibIdx(int&,int&,FILE*&)
       When the final argument is not given, this default constructor
       sets 'nParents' and 'nGroups' to 0 and the two lists to NULL.
       Otherwise, it reads the top of the library, finds and jumps to
       the location of the index, sets 'nParents' and 'nGroups'
       (second argument passed by reference), creates storage for the
       two lists of size 'nParents' and parses the file for to fill
       the lists. 'nParents' and 'nGroups' are sent back to the
       calling routine through the first two reference arguments.

    LibIdx(const LibIdx&)
       The copy constructor does an element-by-element copy of the two
       lists after copying 'nParents'.

    ~LibIdx()
       The inline destructor deletes the storage for both lists.

    LibIdx& operator=(const LibIdx&)
       The correct implementation of this operator must ensure that
       previously allocated space is returned to the free store before
       allocating new space into which to copy the object.
       

    * - Utility - *

    long search(int, int, int)
       This function implements a binary search algorithm on the 'kza'
       list of the LibIdx object.  The first argument is the 'KZA'
       being searched for and the second and third arguments are the
       domain of the search, as indexes to the KZA list.  A search is
       initiated with no indexes (search(int)), and default values
       indicate the beginning of a new search.  Because the KZA list
       is sorted by base ZA number, a few tricks are employed to
       search based on the base ZA number and then scan the
       neighborhood of a match for matching isomeric info.

 * END LibIdx DESCRIPTION *

 *** Class Members ***

 idx : LibIdx*
    A pointer to a LibIdx object which indexes this library.

 binLib : FILE*
    A file pointer for the library itself.

 
 *** Member Functions ***

 * - Constructors & Destructors - *

 ALARALib(char*)
    This default constructor creates a new ALARALib object.  After
    invoking the DataLib constructor in the initialization list, it
    opens the library specified by the filename given in the first
    argument and creates an index, getting the 'nParents' and
    'nGroups' values from the LibIdx constructor.

 ALARALib(const ALARALib&)

    The copy constructor invokes the DataLib copy constructor, copies
    the file pointer and creates a new index from this file.
 
 ~ALARALib()
    The destructor closes the library and deletes the index.

 ALARALib& operator=(const ALARALib&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.

     
 * - Chain - *

 void readData(int, NuclearData*)
    This is the implementation of the readData interface for the
    ALARALib library format.  It searches the 'idx' for the KZA value
    given in the first argument.  It then reads all the data into
    local variables and passes those to NuclearData::setData(...)
    called through the object pointed to by the second argument.  If
    no data is found, it calles NuclearData::setNoData().

 */

#ifndef ALARALIB_H
#define ALARALIB_H

#define DATALIB_ALARA 1
#define DATALIB_GAMMA 5

#include "DataLib/DataLib.h"
#include "ALARALib_def.h"

class ALARALib : public DataLib
{
protected:

  class LibIdx
    {
    protected:
      int nParents, *kza;
      long *offset;
      
    public:
      /* Service */
      LibIdx(int& nPar,int& nGroups, FILE*&,int&);
      LibIdx(const LibIdx&);
      ~LibIdx()
	{ delete kza; delete offset; };

      LibIdx& operator=(const LibIdx&);

      /* Utility */
      long search(int, int min=0, int max=-1);

    } *idx;

  FILE* binLib;
  fstream tmpIdx;
  long offset;


public:
  /* Service */
  ALARALib(char*,int setType=DATALIB_ALARA);
  ALARALib(char*, char*);
  ALARALib(const ALARALib&);
  ~ALARALib();

  ALARALib& operator=(const ALARALib&);

  void close(int,int,char*);
      
  /* Chain */
  void readData(int, NuclearData*);
  void readGammaData(int, GammaSrc*);

  /* Write Binary Data */
  void writeHead(int,float*,float*);
  void writeData(int, int, float, float*, int*, char**, float**);
  void writeGammaData(int,int,int*,int*,int*,float**,float**,int**,int**,
		      float**,float**);
  void appendIdx(char*,int);
  


};


#endif
