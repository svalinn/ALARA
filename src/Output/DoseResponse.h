#include "alara.h"

#ifndef _DOSERESPONSE_H
#define _DOSERESPONSE_H

class DoseResponse
{
protected:
  char *respName;
  union {
    char *fluxName;
    int fluxNum;
  };
  union {
    char *gSrcName;
    GammaSrc *gSrc;
  };
  
  DoseResponse* next;
  
public:
  DoseResponse(char *name=NULL,char* flxName=NULL,char* gammaSrcName=NULL);
  DoseResponse(const DoseResponse&);
  ~DoseResponse() { delete next;};

  DoseResponse& operator=(const DoseResponse&);
  
  DoseResponse* add(char*, char*, char*);
  void xCheck(GammaSrc*, Flux*);
  int getNumGroups();

  char *getName() { return respName; };

  DoesResponse* advance() { return next;};
  
};

#endif
