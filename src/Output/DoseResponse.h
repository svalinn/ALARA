#include "alara.h"

#ifndef _DOSERESPONSE_H
#define _DOSERESPONSE_H

#define DOSE_HEAD 0
#define DOSE_TYPE 1

class DoseResponse
{
protected:
  int format, skip;
  double scale;
  char *respName, *fileName;
  
  DoseResponse* next;
  
public:
  DoseResponse(int type=DOSE_HEAD);
  DoseResponse(const DoseResponse&);
  
  DoseResponse& operator=(const DoseResponse&);
  
  DoseResponse* add(char*, char*, double, int, char*);
  DoseResponse* copy(DoseResponse*);
  
};

#endif
