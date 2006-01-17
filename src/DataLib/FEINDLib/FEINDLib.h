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
  FEINDLib(char* arg0,char* arg1,char* arg2, int setType=DATALIB_FEIND);

  void readData(int, NuclearData*);
  void readGammaData(int, GammaSrc*);
};

#endif
