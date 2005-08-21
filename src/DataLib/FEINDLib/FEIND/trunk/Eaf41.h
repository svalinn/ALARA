#ifndef __EAF41_H
#define __EAF41_H

#include <string>
#include <fstream>

#include "FeindNs.h"
#include "Parent.h"
#include "exception/ExInclude.h"
#include "Parser.h"

/// The parser for the EAF 4.1 data format
/** This library simply contains transmutation cross-sections, including some
 *  fission cross-sections.
 */
class FEIND::Eaf41 : public Parser
{
 public:
  Eaf41(const LibDefine& lib);
  virtual void LoadLibrary() throw(ExFileOpen, ExEmptyXSec);

 private:
  std::string FileName;
  std::ifstream InFile;
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
  int FissionType(const char projectile);

  /// Add the secondary production cross-sections for the charged particles.
  void AddCPPCS(Kza parent, Path& path) throw(ExEmptyXSec);

  std::vector<Kza> Cpt;
};

#endif
