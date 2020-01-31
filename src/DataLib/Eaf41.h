#ifndef EAF41_H
#define EAF41_H

#include <string>
#include <fstream>

#include "FeindNs.h"
#include "Parent.h"
#include "ExInclude.h"
#include "Parser.h"

/// The parser for the EAF 4.1 data format
/** This library simply contains transmutation cross-sections, including some
 *  fission cross-sections.
 */
class FEIND::Eaf41 : public Parser
{
 public:
  /// The main constructor
  /** param[in] lib
   *  Data structure to store the library format and arguments. This parser
   *  takes only a single argument, the path to the nuclear data file.
   */
  Eaf41(const LibDefine& lib);

  /// Function to load data from files into the RamLib
  virtual void LoadLibrary() throw(ExFileOpen, ExEmptyXSec);

 private:

  /// The name of the data file
  std::string FileName;

  /// A file stream associated with the data file
  std::ifstream InFile;

  /// The number of energy groups
  /** As far as I know, Eaf 4.1 files all use the 175 group VITAMIN-J structure
   */
  const static int NumGroups;

  /// Convert the path string into a FEIND path object
  /** The EAF 4.1 data format stores the paths as strings. This function
   *  converts these strings into a path object. An example string is shown
   *  below. \n \code
   *  Li-6 (N,2NA) H-1 \endcode
   */
  Path ConvertPath(const std::string& str);

  /// Convert a character particle identifier into a KZA
  int ParticleEtoF(const char part);

  /// Convert a daughter string into a KZA
  Kza DaughterEtoF(const std::string& daughter);

  /// Skip the header information at the beginning of the file
  void SkipHeader();

  /// Determine the fission type from the projectile
  XSecType FissionType(const char projectile);

  /// Add the secondary production cross-sections for the charged particles.
  void AddCPPCS(Kza parent, Path& path) throw(ExEmptyXSec);

  std::vector<Kza> Cpt;
};

#endif
