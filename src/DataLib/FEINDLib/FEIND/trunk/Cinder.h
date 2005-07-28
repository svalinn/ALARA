#ifndef __CINDER_H
#define __CINDER_H

#include <fstream>
#include <vector>

#include <iostream>
#include "FeindNs.h"

/// The FEIND parser for CINDER format files
/** The Cinder format contains fission yields, cross-sections and decay data.
 *  Only certain cross-sections are primary as well.
 */
class FEIND::Cinder
{
 public:
  Cinder(const LibDefine& lib);
  ErrCode LoadLibrary();
  
 private:
  std::ifstream InFile;

  //*** FUNCTIONS RELATED TO LOADING ACTIVATION DATA ***//

  /// Load cross-sections and decay data
  void ActivationData();

  /// Get the parent kza
  Kza ExtractActParent();

  /// Check to see if the currently examied kza has a non-zero fission xs
  bool CheckFission(const std::string& str);

  /// Build a path object for a reaction
  Path Cinder::MakePath(const std::string& str);

  /// IsPri compares the parent/daughter kzas to determine if this is a primary
  /// daughter
  /** The daughter is primary if its weight is half more than the weight of
   *  the parent.
   */
  bool IsPri(Kza parent, Kza daughter);

  /// Add newCs to oldCs
  /** If oldCs has a different size than newCs, it will be resized.
   */
  void AddToCs(Kza parent_kza, std::vector<double>& newCs);



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
  unsigned int NumNeutronGroups;

  /// The number of gamma groups for gamma spectra
  unsigned int NumGammaGroups;

  /// The neutron energy group boundaries
  /** NeutronGroupBounds.size() = NumNeutronGroups + 1
   */
  std::vector<double> NeutronGroupBounds;

  /// The gamma energy group boundaries
  /** GammaGroupBounds.size() = NumGammaGroups + 1
   */
  std::vector<double> GammaGroupBounds;
};

#endif
