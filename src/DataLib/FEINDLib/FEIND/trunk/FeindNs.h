#ifndef __FEINDNS_H
#define __FEINDNS_H

#include <map>
#include <utility>
#include <string>
#include <vector>

namespace FEIND
{
  #include "Typedefs.h"
  #include "ClassDec.h"
  #include "Consts.h"

  extern RamLib Library;

  extern FissionType DefaultFT;

  extern XSec NULLCS;

  // Functions to query the element list (Elements)
  extern std::pair<std::string,int> Elements[NUM_ELEMENTS];
  int GetAtomicNumber(const std::string& symbol);
  const std::string GetAtomicSymbol(int atomicNumber);

  // Functions to load built in energy group structures:
  /// Simple functions that assigns values to GroupStructs;
  void LoadStructs();
  extern std::map<int, std::vector<double> > GroupStructs;


  

  bool ParentExists(Kza parent);
  ErrCode LoadLibrary(const LibDefine& lib);
  Kza DecayModetoKza(int decayMode, int dIso, Kza parent, Kza& sec);
  

};

#endif
