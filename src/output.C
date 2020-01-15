/* $Id: output.C,v 1.2 1999-08-24 22:06:26 wilson Exp $ */
#include "alara.h"

char* symbol(int Z, char* sym)
{

  strncpy(sym,SYMBOLS+(Z-1)*3+1,2);
  sym[2] = '\0';
  if (sym[1] == ' ')
    sym[1] = '\0';
  return sym;
}

char* isoName(int kza,char* isoSym)
{
  char isomer = 'l'+kza%10;
  kza /= 10;
  sprintf(isoSym,"%s-%d%c",symbol(kza/1000,isoSym),kza%1000,
	  (isomer>'l')?isomer:' ');

  return isoSym;
}
