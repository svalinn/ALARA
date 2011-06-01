#ifndef __FEINDNS_H
#define __FEINDNS_H

/** \file FeindNs.h
 *  \brief This file stores the declaration of the FEIND namespace. 
 *
 *  User applications should not include this file, but should instead include 
 *  FEIND.h
 */

#include <map>
#include <utility>
#include <string>
#include <vector>

/// The FEIND namespace
/** This namespace contains everything users need to make use of FEIND.
 *  However, it contains many common names, and users should generally avoid
 *  importing the entire FEIND namespace into their applications to avoid
 *  conflicts.
 */
namespace FEIND
{
  /// Kza are numbers which uniquely identify a particular isotope.
  /** Kzas are calulated using the atomic number, mass number, and isomeric
   *  state number of a given isotope using the following formula:
   *  \f[ KZA = (10000 \times Z) + (10 \times A) + I \f]
   *  where:\n
   *  \f$ Z \f$ is the atomic number of the isotope\n
   *  \f$ A \f$ is the mass number of the isotope\n
   *  \f$ I \f$ is a number representing the isomeric state
   */
  typedef int Kza;

  #include "ClassDec.h"
  #include "Consts.h"

  /// The main data structure that is used to store nuclear data in FEIND
  /** Library is a RamLib object that users interact with when they need access
   *  to nuclear data. Only one Library exists in FEIND, because of this, only
   *  one copy of each type of data can be stored at a given time.
   */
  extern RamLib Library;

  /// Default fission type used in FEIND
  /** This variable is used to simplify the process of dealing with fission
   *  yields. Functions such as RamLib::LambdaEff(...) use this variable when 
   *  users do not provide information about which type of fission yields to 
   *  use. This variable is initialized to NO_FISSION. Without changing it, or 
   *  explicitely using the a different value when interacting with FEIND, 
   *  fission yields will not be used in data calculations, even if they are 
   *  loaded into the Library!
   */
  extern FissionType DefaultFT;

  /// A cross-section object used to identify an empty cross-section.
  /** NULLCS is a variable that is returned from some RamLib functions that 
   *  can be used in boolean expressions to ensure that a variable exists.
   *  for example:
   *  \code
   *  XSec u235_total = FEIND::Library.GetPCs(922350, TOTAL_CS);
   *  
   *  if(u235_total)
   *  {
   *     ...
   *  }
   *  \endcode
   *  It is important to check that a cross-section exists, because most
   *  XSec member functions will throw exceptions if that cross-section is not
   *  set.
   */
  extern const XSec NULLCS;

  /// An object that is used to represent an empty vector of kzas
  extern const std::vector<Kza> EMPTY_VEC_KZA;

  /// An object used to represent an empty pair of doubles
  extern const std::vector<std::pair<double,double> > EMPTY_PAIR_DOUBLE;

  /// Object to map atomic numbers to atomic symbols
  extern std::pair<std::string,int> Elements[NUM_ELEMENTS];

  /// Retrieve the atomic number associated with an atomic symbol
  /** \param[in] symbol
   *  The atomic symbol should have the first character capitalized and the
   *  second lower case.
   *  
   *  \return 
   *  This function returns the atomic number
   */
  int GetAtomicNumber(const std::string& symbol);

  /// Retrieve the atomic symbol associated with an atomic number
  /** \param[in] atomicNumber
   *  The atomic number of the atom
   *
   *  \return
   *  Return a string containing the atomic symbol. This string contains
   *  at most two characters, the first of which is capitalized, and the second
   *  of which is lower case.
   */
  const std::string GetAtomicSymbol(int atomicNumber);

  /// Load a nuclear data library into memory
  /** \param[in] lib
   *  Data structure containing the format of the library, and the libraries
   *  argument list.
   */
  void LoadLibrary(const LibDefine& lib);

  /// Convert a DecayMode into a daughter nuclide, and secondary particle
  /** This function accepts a decay mode type, parent kza, and isomeric state
   *  identifier to determine the Kza of the decay daughter, and the Kza
   *  of any secondary particles that may be produced.
   *
   *  \param[in] decayMode
   *  Type of decay that is occurring
   *
   *  \param[in] dIso
   *  Isomeric state of the daughter nuclide
   *
   *  \param[in] parent
   *  Isomeric state of the parent nuclide
   *
   *  \param[out] sec
   *  Kza of a secondary particle, if it is produced. The value of sec will
   *  only be set for heavy charged particles. Otherwise the value will be
   *  set to zero.
   *
   *  \return
   *  This function returns the Kza of the daughter nuclide.
   */
  Kza DecayModetoKza(DecayModeType decayMode, int dIso, Kza parent, Kza& sec);
};

#endif
