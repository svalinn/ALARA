#ifndef __ENDFIEAF_H
#define __ENDFIEAF_H

#include <vector>
#include <string>
#include <fstream>

#include "FeindNs.h"

class FEIND::EndfIeaf
{
 public:
  EndfIeaf(const LibDefine& lib);
  ErrCode LoadLibrary();

 private:
  std::ifstream InFile;

  bool Is35(const std::string& str);
  void ExtractCs(Kza parent, Kza daughter, int num);
};

#endif
