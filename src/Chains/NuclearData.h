/* $Id: NuclearData.h,v 1.16 2003-01-13 04:34:52 fateneja Exp $ */
#include "alara.h"

#ifndef NUCLEARDATA_H
#define NUCLEARDATA_H

/** \brief Stores the nuclear data information for a particular isotope 
           in the chain.
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

  /// The number of reactions to be processed for this isotope.
  /** This can be interpreted physically in a number of ways depending
      on the library structure, but computationally it represents the
      number of different daughters (or parents) that can be added to a
      give node. */
  int nPaths;

  /// The number of reactions contained in the library for this isotope.
  /** This is always less than or equal to nPaths, which is modified
      by stripNonDecay() below. */
  int origNPaths;

  /// This is an array of KZA values for the daughter products (or
  /// parents) of the NuclearData::nPaths reaction paths.
  int *relations;

  /// This is an array of character strings indicating the type of
  /// reaction/emitted particles for each reaction path.
  char **emitted;

  /// This is a rate vector describing the "single" transfer rate
  /// related to the "previous" isotope by both transmutation (groups
  /// 1..N) and decay (group N+1).
  /** For forward calculations, this is the production rate, and for
      backward, it is the total destruction rate. */
  double *single;
  
  /** This is an array of rate vectors describing the rates related to
      the NuclearData::nPaths "next" isotopes by both transmutation
      (groups 1..N) and decay (group N+1) for each reaction and the
      total reaction rate. */
  double **paths;
    
  /// This stores the average beta, gamma and alpha energy per
  /// nuclear decay.
  double E[3];

  /// This will point to the production rate, whichever that happens
  /// to be.
  double *P;
 
  /// This will be set to point to the destruction rate, whichever
  /// that happens to be.
  double *D;

  /// {Chain Building} This function checks all the reactions and
  /// strips the ones that contain no decay rate.
  int stripNonDecay();

  /// {Chain Building} This function sorts all the reaction path data,
  /// moving the decay reaction paths to the beginning of the list.
  void sortData();

 public:

  /// {Input Handling} This function reads the library type and calls for the
  /// creation of a new DataLib object.
  static void getDataLib(istream&);

  /// {Service/Cleanup} This function simply deletes the static member 
  /// 'dataLib'.
  static void closeDataLib();

  /// {Input Handling} This inline function just sets NuclearData::mode
  /// variable into the reverse mode.
  static void modeReverse() { mode = MODE_REVERSE;};

  /// {Service/Access} This inline function provides access to 
  /// NuclearData::mode
  static int getMode() { return mode; };

  /*
    NuclearData(double*)
    The basic constructor is used when adding new Node's to the chain.
    After initializations identical to the default constructor, it
    creates and fills the storage for 'P' by copying the passed array
    on an element-by-element basis.
  */

  /// The default constructor.
  NuclearData();

  /// The copy constructor.
  NuclearData(const NuclearData&);

  ///  This destructor deletes all the storage associated with the
  ///  current NuclearData object.
  ~NuclearData();

  /// Overloaded assignment operator.
  NuclearData& operator=(const NuclearData&);

  
  /// This function performs all the memory allocation required when
  /// deleting or resetting the values of a NuclearData object.
  void cleanUp();
  
  /// {%Chain Building} Callback function to pass data back from library 
  /// routines.
  void setData(int, float *, int *, char **, float **, float, float*);

};



#endif
