/* $Id: ADJLib.h,v 1.5 1999-08-24 22:06:18 wilson Exp $ */
#include "alara.h"
/* ******* Class Description ************

This class provides access to data libraries which follow the ALARA v1
binary merged library format.  This class is derived directly and
publicly from class DataLib.

*** Supported Data Library Formats ***

This catalogue of data library types should be included in every new
module developed to support a new data library format.

                       Input
Identifier     Value   String   Description
-------------------------------------------------------------------
DATALIB_NULL     0     null     A basic DataLib object
                                (should rarely be used in final object)
DATALIB_ALARA    1     alara    The default ALARA v1 binary format.
DATALIB_ASCII    2     ascii    A basic ASCII DataLib object
                                (should rarely be used in final object)
DATALIB_EAF      3     eaf      A data library following the formatting
                                definition of the EAF library (roughly
                                ENDF/B-6) 
DATALIB_ADJOINT  4     adj      An alara binary library in reversed format
                                for reverse calculations.
DATALIB_GAMMA    5     gamma    An alara binary library containing gamma
                                source information.
-------------------------------------------------------------------
*/

#ifndef _ADJLIB_H
#define _ADJLIB_H

#include "ALARALib.h"


class ADJLib : public ALARALib
{
protected:

  /* member class */
  class DaugItem
    {
    protected:

      /* member's member class */
      class ParItem
	{
	protected:
	  int kza;
	  long offset;
	  
	  ParItem* next;

	  ParItem(ParItem*);

	public:
	  ParItem(int,long);
	  void add(int,long);
	  int count();
	  ParItem* advance() { return next;};
	  int getKza() { return kza;};
	  long getOffset() { return offset; };
	}
      *parList, *current;
      
      int kza;

      DaugItem *next;
      DaugItem(DaugItem*);

    public:
      DaugItem(int,int,long);
      void add(int,int,long);
      int count();
      DaugItem* advance() { return next; };
      int getKza() { return kza; };
      int countRxns() { return parList->count(); };
      long getNextReaction(int&);
    }
  *daugList;
  
  FILE* normBinLib;

  float *totalXSection, E[3], thalf;
  float *xSection;
  char emitted[32];

  void copyHead();
  void getForwardData(int);
  void writeData(DaugItem*);
  void build();
  
public:
  ADJLib(char*);
  ADJLib(char*,char*);

  
};
  
#endif
