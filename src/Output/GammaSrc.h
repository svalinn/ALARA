/* $Id: GammaSrc.h,v 1.3 2000-06-20 18:31:43 wilson Exp $ */
#include "alara.h"
#include <set>

/* ******* Class Description ************

 */

#ifndef _GAMMASRC_H
#define _GAMMASRC_H


class GammaSrc
{
protected:
  
  char *fileName;
  int nGroups;
  double *grpBnds;
  ofstream gSrcFile;

public:

};

#endif
