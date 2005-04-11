#ifndef __DECAYENDF6_H
#define __DECAYENDF6_H

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <utility>

#include "FeindNs.h"
#include "Parent.h"

class FEIND::DecayEndf6
{
 public:
  DecayEndf6(const LibDefine& lib);
  ErrCode LoadLibrary();

  static double FormatFloat(std::string str);

 private:
  std::ifstream InFile;

  Kza ExtractParent(const std::string& str);
  double ExtractDecayConst(const std::string& str);
  void ExtractEnergies(Kza parent, std::string& str);
  ErrCode ExtractDecayModes(Kza parent);
  ErrCode ExtractSpectrum(unsigned int num, Kza parent);
  std::vector< std::pair<double, double> > ExtractPairs(int num);
  bool Is8457(const std::string& str);
  int ModeEtoF(double endf);
  int SpectrumEtoF(double endf);
};

#endif
