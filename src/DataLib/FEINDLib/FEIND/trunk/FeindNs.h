#ifndef __FEINDNS_H
#define __FEINDNS_H

#include <map>
#include <utility>
#include <string>
#include <vector>

namespace FEIND
{
  typedef int Kza;
  #include "ClassDec.h"
  #include "Consts.h"

  extern RamLib Library;

  extern FissionType DefaultFT;
  extern const XSec NULLCS;
  extern const std::vector<Kza> EMPTY_VEC_KZA;
  extern const std::vector<std::pair<double,double> > EMPTY_PAIR_DOUBLE;

  // Functions to query the element list (Elements)
  extern std::pair<std::string,int> Elements[NUM_ELEMENTS];
  int GetAtomicNumber(const std::string& symbol);
  const std::string GetAtomicSymbol(int atomicNumber);
  void LoadLibrary(const LibDefine& lib);
  Kza DecayModetoKza(DecayModeType decayMode, int dIso, Kza parent, Kza& sec);
  

};

#endif
