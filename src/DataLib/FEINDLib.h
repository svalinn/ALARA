#ifndef FEINDLIB_H
#define FEINDLIB_H

#define DATALIB_FEIND 7

#include "DataLib.h"
#include "FEIND.h"
#include <vector>

using namespace std;

enum FissionType {NO_FISSION, FAST, THERMAL, HOT, SF};

class FEINDLib : public DataLib
{ 
 public:
  FEINDLib(char* arg0,char* arg1,char* arg2, int setType=DATALIB_FEIND);

  void readData(int, NuclearData*);
  void readGammaData(int, GammaSrc*);

 private:
  FissionType fissionType;
  void initFissionType(char* arg2);

};

#endif
