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
-------------------------------------------------------------------
*/

#include "ALARALib.h"

#define DATALIB_ADJOINT 4

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
	public:
	  ParItem(int,long,ParItem*);
	  void add(int,long);
	  int count();
	  ParItem* advance() { return next;};
	  int getKza() { return kza;};
	  long getOffset() { return offset; };
	}
      *parList, *current;
      
      int kza;

      DaugItem *next;

    public:
      DaugItem(int,DaugItem*);
      void add(int,int,long);
      int count();
      DaugItem* advance() { return next; };
      int getKza() { return kza; };
      int countRxns() { return parList->count(); };
      long getNextReaction();
    }
  *daugList;
  
  FILE* normBinLib;

  float *totalXSection, E[3], thalf;
  float *xSection;
  char *emitted;

  void copyHead();
  void getForwardData(int);
  void writeData(DaugItem*);
  void build();
  
public:
  ADJLib(char*);
  ADJLib(char*,char*);

  
};
  
