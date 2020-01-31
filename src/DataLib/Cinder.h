#ifndef CINDER_H
#define CINDER_H

#include <fstream>
#include <vector>
#include <iostream>
#include <string>

#include "FeindNs.h"
#include "exception/ExInclude.h"
#include "Parser.h"

/// The FEIND parser for CINDER format files
/** The Cinder format contains fission yields, cross-sections and decay data.
 */
class FEIND::Cinder : public Parser
{
 public:

  /// Primary constructor for the FEIND parser
  /** \param[in] lib
   *  Structure to store information about the file format, and parser options.
   *  The cinder parser takes several options. The first element of the
   *  lib.Args must be the path to the nuclead data file. Other options
   *  include:
   *    - notrans: indicates that transmutation data (cross-sections) should
   *               not be loaded.
   *    - nodecay: indicates that decay data should not be loaded
   *    - noyields: indicates that fission yields should not be loaded
   */
  Cinder(const LibDefine& lib);

  /// Function to load data into the RamLib.
  virtual void LoadLibrary() throw(Exception);
  
 private:
  /// The input file stream
  std::ifstream InFile;

  /// The name of the data file
  std::string FileName;

  //*** ARGUMENT HANDLING ***//

  /// Flag to indicate whether transmutation data should be loaded
  bool LoadTransmutation;

  /// Flag to indicate whether decay data should be loaded
  bool LoadDecay;

  /// Flag to indicate whether fission yields should be loaded.
  bool LoadYields;

  //*** FUNCTIONS RELATED TO LOADING ACTIVATION DATA ***//

  /// Load cross-sections and decay data
  void ActivationData() throw(Exception);

  /// Get the parent kza
  Kza ExtractActParent();

  /// Check to see if the currently examied kza has a non-zero fission xs
  bool CheckFission(const std::string& str);

  /// Build a path object for a reaction
  Path MakePath(const std::string& str);

  /// IsPri compares the parent/daughter kzas to determine if this is a primary
  /// daughter
  /** The daughter is primary if its weight is half more than the weight of
   *  the parent.
   */
  bool IsPri(Kza parent, Kza daughter);
  
  /// This function is responsible for loading decay data.
  /** This can be prevented by sending CINDER the "nodecay" argument.
   */
  void DecayData(Kza parent) throw(ExDecayMode);


  //*** FUCNTIONS RELATED TO LOADING FISSION YIELDS ***//

  /// Load Fission Yields
  void FissionYields();

  /// This function returns a list of all fissionable isotopes in the library
  void ExtractFissionParents(int num, std::vector<Kza>& parents,
                             std::vector<char>& fissionType);

  /// This function simply skips the atomic masses that are listed in the 
  /// library.
  void SkipMasses(int num);

  /// Return the yields for each daughter
  std::vector<double> ExtractYield(int num, Kza& daughter);

  /// Determine the type of fission associated with each isotope
  /** Fast, thermal, ... 
   */
  int FissionType(char t);

  

  //*** OTHER FUCNTIONS ***//

  /// Get number of groups and group boundaries
  void GetGroupInfo();

  /// This function converts CINDER style kzas to FEIND style kzas
  /** CINDER style Kzas are stored as:\n
   *    A * 1000 + Z * 10 + M\n
   *  Whereas FEIND style are stored as:\n
   *    Z * 1000 + A * 10 + M
   */
  Kza CinderToKza(int cinder);

  /// The number of neutron energy groups for the cross-sections
  int NumNeutronGroups;

  /// The number of gamma groups for gamma spectra
  int NumGammaGroups;

  /// The neutron energy group boundaries
  /** NeutronGroupBounds.size() = NumNeutronGroups + 1
   */
  std::vector<double> NeutronGroupBounds;

  /// The gamma energy group boundaries
  /** GammaGroupBounds.size() = NumGammaGroups + 1
   */
  std::vector<double> GammaGroupBounds;

  /// Function to determine the decay mode from the parent and daughter.
  DecayModeType KzaToDecayMode(Kza parent, Kza daughter);

  
  /// The upper limit for half lives
  /** Instead of using some special symbol to indicate that certain isotopes
   *  are stable, the CINDER just sets the half life to a really big number.
   *  CINDER_UPPER_HL is compared against to determine whether this is the
   *  case.
   */
  const static double CINDER_UPPER_HL;
  
  /// The number of digits CINDER files use to represent floating point numbers
  const static int FLOAT_DIGITS;
};

#endif
