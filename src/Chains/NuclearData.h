#include "alara.h"

/* ******* Class Description ************

This class stores the nuclear data information for a particular
isotope in the chain.  It is not intended that there be an instance of
class NuclearData, but just that is serves as a base class for class
Node.

 *** Static Class Members ***

 nGroups : int
    The number of groups in the data library.

 dataLib : DataLib*
    This is a pointer to the data library being used in this problem.
    The DataLib class is a base class for an extensible hierarchy of
    derived classes which represent various formats of data library,

 *** Class Members ***

 nPaths : int
    The number of reactions to be processed for this isotope.  This
    can be interpreted physically in a number of ways depending on the
    library structure, but computationally it represents the number of
    different daughters (or parents) that can be added to a give node.

 relations : int*
    This is an array of KZA values for the daughter products (or
    parents) of the 'nPaths' reaction paths.

 emitted : char**
    This is an array of character strings indicating the type of
    reaction/emitted particles for each reaction path.

 single : double*
    This is a rate vector describing the "single" transfer rate
    related to the "previous" isotope by both transmutation (groups
    1..N) and decay (group N+1).  For forward calculations, this is
    the production rate, and for backward, it is the total destruction
    rate.

 paths : double**
    This is an array of rate vectors describing the rates related to
    the 'nPaths' "next" isotopes by both transmutation (groups 1..N)
    and decay (group N+1) for each reaction and the total reaction
    rate.

 E[3] : double
    This stores the average beta, gamma and alpha energy per nuclear
    decay.

 P : double *
    This will point to the production rate, whichever that happens to
    be.

 D : double *
    This will be set to point to the destruction rate, whichever that
    happens to be.

 *** Protected Member Functions ***

 * - Chain - *

 int stripNonDecay() 
    This function checks all the reactions and strips the ones that
    contain no decay rate.  Both the 'nPaths' member and the related
    arrays ('d','emitted','daughters') are reduced to reflect the new
    number of reaction paths.  This is used when the truncation state
    has determined that at most radioactive branches should be
    considered in the chain creation process.  After counting the
    radioactive decay branches, it returns an adjusted truncation
    state.

 *** Public Static Member Funtions ***

 * - Input - *

 void getDataLib(istream&)
    This function read the library type from the input file attached
    to the stream reference argument and calls for the creation of a
    new DataLib object through the DataLib::newLib(...) function.  It
    also requests the number of groups from the dataLib to set its own
    static member 'nGroups' and share the info with other classes
    (e.g. VolFlux).

 * - Constructors & Destructors - *

 NuclearData()
    The default constructor initializes 'nPaths' to -1, for use later,
    sets all the pointers to NULL, and zeroes the E[] array.

 NuclearData(const NuclearData&)
    The copy constructor copies all the data on an element-by-element
    basis, allocating new storage where necessary.  It does NOT simply
    copy pointers.

 NuclearData(double*)
    The basic constructor is used when adding new Node's to the chain.
    After initializations identical to the default constructor, it
    creates and fills the storage for 'P' by copying the passed array
    on an element-by-element basis.

 ~NuclearData()
    This destructor deletes all the storage associated with the
    current Node object.

 * - Chain - *

 void setData(int, float *, int *, char **, float **)
    The arguments for this function are the data as read from the
    library (Note that single precision is sufficient for library
    data).  These data are copied into 'nPaths', 'E',
    'daughters','emitted', and 'd', respectively.

 void setNoData()
    This inline function sets the number of reaction paths to 0.  The
    initial/default value is -1, which is a flag to search for the
    data.  If no data is found, this must be set to 0 to indicate that
    data was searched for an there are no reaction paths.

 */

#ifndef _NUCLEARDATA_H
#define _NUCLEARDATA_H


class NuclearData
{
protected:
  static int nGroups;
  static DataLib *dataLib;
  static int mode;

  int nPaths, *relations;
  char **emitted;
  double *single;
  double **paths, E[3];
  double *P, *D;

  /* Chain */
  int stripNonDecay();

 public:

  /* Input */
  static void getDataLib(istream&);
  static void closeDataLib();
  static void modeReverse() { mode = MODE_REVERSE;};
  static int getMode() { return mode; };
  static int getNumGroups() { return nGroups; };

  /* Service */
  NuclearData();
  NuclearData(const NuclearData&);
  ~NuclearData();

  NuclearData& operator=(const NuclearData&);
  
  void cleanUp();

  /* Chain */
  void setData(int, float *, int *, char **, float **, float, float*);
};



#endif
