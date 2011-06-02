#ifndef ENDFIEAF_H
#define ENDFIEAF_H

#include <vector>
#include <string>
#include <fstream>

#include "FeindNs.h"
#include "Parser.h"
#include "exception/Exception.h"

/// Parser for loading cross-sections from ENDF-VI files
class FEIND::EndfIeaf : public Parser
{
 public:
  EndfIeaf(const LibDefine& lib);
  virtual void LoadLibrary() throw(ExFileOpen, ExEmptyXSec);

 private:
  std::ifstream InFile;
  std::string FileName;

  bool Is35(const std::string& str);
  void ExtractCs(Kza parent, Kza daughter, int num) throw(ExEmptyXSec);
};

#endif
