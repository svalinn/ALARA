#ifndef DECAYENDF6_H
#define DECAYENDF6_H

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <utility>

#include "FeindNs.h"
#include "Parent.h"
#include "ExInclude.h"
#include "Parser.h"

/// Parser for loading decay data from ENDF-VI files
class FEIND::DecayEndf6 : public Parser
{
 public:
  DecayEndf6(const LibDefine& lib);
  virtual void LoadLibrary() throw(ExFileOpen, ExDecayMode);

  static double FormatFloat(std::string str);

 private:
  std::ifstream InFile;
  std::string FileName;

  Kza ExtractParent(const std::string& str);
  double ExtractDecayConst(const std::string& str);
  void ExtractEnergies(Kza parent, std::string& str);
  void ExtractDecayModes(Kza parent) throw(ExDecayMode);
  void ExtractSpectrum(unsigned int num, Kza parent);
  std::vector< std::pair<double, double> > ExtractPairs(int num);
  bool Is8457(const std::string& str);
  DecayModeType ModeEtoF(double endf);
  int SpectrumEtoF(double endf);
};

#endif
