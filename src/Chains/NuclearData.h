/* $Id: NuclearData.h,v 1.15 2002-08-05 20:23:14 fateneja Exp $ */
#include "alara.h"



/*

 *** Protected Member Functions ***

 * - Service - *

 void cleanUp() 



 */

#ifndef _NUCLEARDATA_H
#define _NUCLEARDATA_H

/** \brief Stores the nuclear data information for a particular isotope in the chain.
 *
 *  It is not intended that there be an instance of class NuclearData,
 *  but just that is serves as a base class for class Node.
 */

class NuclearData
{
protected:
  /// This is a pointer to the data library being used in this problem.
  /** The DataLib class is a base class for an extensible hierarchy of
      derived classes which represent various formats of data library   */
  static DataLib *dataLib;

  /// This flag defines whether this is a forward or reverse calculation.
  /** This flag is the basis for whether the forward (ALARALib) or 
      reverse (ADJLib) nuclear data library is used.*/
  static int mode;

  /// The number of groups in the data library.
  static int nGroups;

  int
    /// The number of reactions to be processed for this isotope.
    /** This can be interpreted physically in a number of ways depending
	on the library structure, but computationally it represents the
	number of different daughters (or parents) that can be added to a
	give node. */
    nPaths,

    /// The number of reactions contained in the library for this isotope.
    /** This is always less than or equal to nPaths, which is modified
	by stripNonDecay() below. */
    origNPaths,

    /// This is an array of KZA values for the daughter products (or
    /// parents) of the NuclearData::nPaths reaction paths.
    *relations;

  char
    /// This is an array of character strings indicating the type of
    /// reaction/emitted particles for each reaction path.
    **emitted;

  double
    /// This is a rate vector describing the "single" transfer rate
    /// related to the "previous" isotope by both transmutation (groups
    /// 1..N) and decay (group N+1).
    /** For forward calculations, this is the production rate, and for
	backward, it is the total destruction rate. */
    *single,

    /** This is an array of rate vectors describing the rates related to
	the NuclearData::nPaths "next" isotopes by both transmutation
	(groups 1..N) and decay (group N+1) for each reaction and the
	total reaction rate. */
    **paths,
    
    /// This stores the average beta, gamma and alpha energy per
    /// nuclear decay.
    E[3],

    /// This will point to the production rate, whichever that happens
    /// to be.
    *P, 
    /// This will be set to point to the destruction rate, whichever
    /// that happens to be.
    *D;


  /// {Chain Building} This function checks all the reactions and
  /// strips the ones that contain no decay rate.
  /**  Both the NuclearData::nPaths member and the related arrays
    (NuclearData::paths, NuclearData::emitted, and NuclearData::relation)
    are reduced to reflect the new number of reaction paths.  This is
    used when the truncation state has determined that at most
    radioactive branches should be considered in the chain creation
    process.  After counting the radioactive decay branches, it
    returns an adjusted truncation state. */
  int stripNonDecay();

  /// {Chain Building} This function sorts all the reaction path data,
  /// moving the decay reaction paths to the beginning of the list.
  /** This change was necessary to enable the RateCache concept to
      work since it is necessary to have the reactions indexed the
      same way, even if non-decay reactions have been stripped from
      the list.  After sorting, decay reaction 1...d will always be
      1...d whether or not the non-decay reactions are present. */
  void sortData();

 public:

  /// {Input Handling} This function reads the library type and calls for the creation of a new DataLib object.
  /** This function reads the library type from the input file attached
    to the stream reference argument and calls for the creation of a
    new DataLib object through the DataLib::newLib(...) function.  It
    also requests the number of groups from the dataLib to set its own
    static member 'nGroups' and share the info with other classes
    (e.g. VolFlux). */
  static void getDataLib(istream&);

  /// {Service/Cleanup} This function simply deletes the static member 'dataLib'.
  static void closeDataLib();

  /// {Input Handling} This inline function just sets NuclearData::mode
  /// variable into the reverse mode.
  static void modeReverse() { mode = MODE_REVERSE;};

  /// {Service/Access} This inline function provides access to NuclearData::mode
  static int getMode() { return mode; };

  /*
    NuclearData(double*)
    The basic constructor is used when adding new Node's to the chain.
    After initializations identical to the default constructor, it
    creates and fills the storage for 'P' by copying the passed array
    on an element-by-element basis.
  */

  /// The default constructor.
  /** The default constructor initializes NuclearData::nPaths to -1,
    for use later, sets all the pointers to NULL, and zeroes the NuclearData::E[]
    array. */
  NuclearData();

  /// The copy constructor.
  /** The copy constructor copies all the data on an
    element-by-element basis, allocating new storage where necessary.
    It does \b NOT simply copy pointers. */
  NuclearData(const NuclearData&);

  ///  This destructor deletes all the storage associated with the
  ///  current NuclearData object.
  ~NuclearData();

  /// Overloaded assignment operator.
  /** The assignment operator copies all the data on an
    element-by-element basis, freeing old storage and allocating new
    storage where necessary.  It does \b NOT simply copy pointers. */
  NuclearData& operator=(const NuclearData&);

  
  /// This function performs all the memory allocation required when
  /// deleting or resetting the values of a NuclearData object.
  /** This task might be required in a variety of places so the
      functionality was encapsulated into a single function. */
  void cleanUp();
  
  /// {%Chain Building} Callback function to pass data back from library routines.
  /** This function implements a callback from the nuclear data
    library modules.  The arguments for this function are the data as
    read from the library (Note that single precision is sufficient
    for library data).  These data are copied into
    NuclearData::nPaths, NuclearData::E, NuclearData::relations,
    NuclearData::emitted, NuclearData::paths, NuclearData::D[ngroups],
    and NuclearData::single respectively. */
  void setData(int, float *, int *, char **, float **, float, float*);

};



#endif
