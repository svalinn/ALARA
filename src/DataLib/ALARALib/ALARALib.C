/* File sections:
 * Service: constructors, destructors
 * Lib: functions directly related to library handling
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "ALARALib.h"

/****************************
 ********* Service **********
 ***************************/

ALARALib::ALARALib(char* fname) : DataLib(DATALIB_ALARA)
{
  binLib = fopen(fname,"rb");

  idx = new LibIdx(nParents,nGroups,binLib);
}

ALARALib::ALARALib(const ALARALib& a) : DataLib(a)
{
  binLib = a.binLib;

  idx = new LibIdx(*(a.idx));
}

ALARALib::~ALARALib()
{
  fclose(binLib);
  delete idx;
}

ALARALib& ALARALib::operator=(const ALARALib& a)
{

  return *this;

  if (this == &a)
    return *this;

  binLib = a.binLib;
  
  *idx = *(a.idx);
}

/****************************
 ********** Chain ***********
 ***************************/

/* read data for one isotope and set the NuclearData object */
void ALARALib::readData(int findKza, NuclearData* data)
{
  long offset;
  int checkKza, nRxns;
  float thalf, E[3];

  int rxnNum, emittedLen;

  verbose(4,"Looking for data for %d",findKza);

  /* search index and go to that location*/
  offset = idx->search(findKza);

  if (offset > 0)
    {
      verbose(5,"Found data for %d at offset %d",findKza,offset);
      fseek(binLib,offset,SEEK_SET);
      
      /* get isotope info */
      fread(&checkKza,SINT,1,binLib);
      fread(&nRxns,SINT,1,binLib);
      fread(&thalf,SFLOAT,1,binLib);
      fread(E,SFLOAT,3,binLib);
      
      /* setup arrays */
      int *daugKza= new int[nRxns];
      memCheck(daugKza,"ALARALib::readData(...): daugKza");

      float **xSection = new float*[nRxns];
      memCheck(xSection,"ALARALib::readData(...): xSection");

      char **emitted = new char*[nRxns];
      memCheck(emitted,"ALARALib::readData(...): emitted");
      
      /* Read info for each daughter */
      for (rxnNum=0;rxnNum<nRxns;rxnNum++)
	{
	  fread(daugKza+rxnNum,SINT,1,binLib);
	  fread(&emittedLen,SINT,1,binLib);

	  emitted[rxnNum] = new char[emittedLen+1];
	  memCheck(emitted[rxnNum],"ALARALib::readData(...): emitted[n]");
	  fread(emitted[rxnNum],1,emittedLen,binLib);
	  emitted[rxnNum][emittedLen] = '\0';

	  xSection[rxnNum] = new float[nGroups+1];
	  memCheck(xSection[rxnNum],"ALARALib::readData(...): xSection[n]");
	  fread(xSection[rxnNum],SFLOAT,nGroups+1,binLib);
	}
      
      verbose(5,"Read %d reaction path(s) for %d.",nRxns,findKza);
      /* set the NuclearData object */
      data->setData(nRxns,E,daugKza,emitted,xSection,thalf);
      
      delete [] xSection;
      delete [] emitted;
      delete daugKza;
    }
  else
    data->setNoData();
  
}
