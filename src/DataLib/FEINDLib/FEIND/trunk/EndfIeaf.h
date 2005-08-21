#ifndef __ENDFIEAF_H
#define __ENDFIEAF_H

#include <vector>
#include <string>
#include <fstream>

#include "FeindNs.h"
#include "Parser.h"

class FEIND::EndfIeaf : public Parser
{
 public:
  EndfIeaf(const LibDefine& lib);
  virtual void LoadLibrary() throw(ExFileOpen);

 private:
  std::ifstream InFile;
  std::string FileName;

  bool Is35(const std::string& str);
  void ExtractCs(Kza parent, Kza daughter, int num);
};

#endif
