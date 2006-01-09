#ifndef _FEINDLIB_H
#define _FEINDLIB_H

#define DATALIB_FEIND 7

#include "../DataLib.h"
#include "FEIND/FEIND.h"
#include <vector>

using namespace std;

class FEINDLib : public DataLib
{ 
 public:
  FEINDLib(vector<string> filePath, vector<string> fileFormat, int setType=DATALIB_FEIND);

  void readData(int, NuclearData*);
  void readGammaData(int, GammaSrc*);
};

#endif
