#include "alara.h"

#ifndef _GAMMASRC_H
#define _GAMMASRC_H

#include "Input/Input_def.h"

class GammaSrc
{
protected:
  char *name;
  int nGrps;
  double *grpBnds;

  GammaSrc *next;

public:
  GammaSrc(char *setName=IN_HEAD);

  ~GammaSrc() {delete next;};

  GammaSrc* getGammaSrc(istream&);

  GammaSrc* find(char*);
  int getNumGroups() { return nGrps; };
};

#endif

