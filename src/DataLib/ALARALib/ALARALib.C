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

/* create a new library */
ALARALib::ALARALib(char* fname, char* idxName) 
  : DataLib(DATALIB_ALARA)
{
  binLib = fopen(fname,"wb");
  tmpIdx.open(idxName, ios::out);
  offset = 0;
  
  idx = NULL;
}

/* open an existing library */
ALARALib::ALARALib(char* fname) 
  : DataLib(DATALIB_ALARA)
{

  char fnameStr[256];
  strcpy(fnameStr,fname);
  strcat(fnameStr,".lib");

  binLib = fopen(fnameStr,"rb");

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
      
      for (rxnNum=0;rxnNum<nRxns;rxnNum++)
	{
	  delete xSection[rxnNum];
	  delete emitted[rxnNum];
	}
      delete xSection;
      delete emitted;
      delete daugKza;
    }
  else
    data->setNoData();
  
}

/*****************************************
 ********** Binary Library Mgmt **********
 ****************************************/


void ALARALib::writeHead(int readNGrps, float *grpBnds, float *grpWeights)
{

  nGroups = readNGrps;

  /************
   * write head info to binary library 
   ************/
  /* save place for offset of index */
  offset += fwrite(&offset,SLONG,1,binLib)*SLONG;

  /* save place for number of parents */
  offset += fwrite(&nParents,SLONG,1,binLib)*SLONG;

  /* write number of neutron groups */
  offset += fwrite(&nGroups,SINT,1,binLib)*SINT;

  /* write group structure info */
  tmpIdx << -1 << "\t" << offset << endl;
  offset += fwrite(&grpBnds,SINT,1,binLib)*SINT;
  if (grpBnds != NULL)
    offset += fwrite(grpBnds,SFLOAT,nGroups+1,binLib)*SFLOAT;
  tmpIdx << 0 << "\t" << offset << endl;
  offset += fwrite(&grpWeights,SINT,1,binLib)*SINT;
  if (grpWeights != NULL)
    offset += fwrite(grpWeights,SFLOAT,nGroups,binLib)*SFLOAT;
  
  debug(4,"Wrote header info.");


}

/* write the data for a single isotope */
void ALARALib::writeData(int kza, int nRxns, float thalf, float *E,
			 int *daugKza, char **emitted, float **xSection)
{
  int unique, emittedLen;

  verbose(2,"Writing entry for %d (%d)",kza,offset);

  /* write parent isotope info */
  tmpIdx << kza << "\t" << nRxns << "\t" << offset << endl;
  offset += fwrite(&kza,SINT,1,binLib)*SINT;
  offset += fwrite(&nRxns,SINT,1,binLib)*SINT;
  offset += fwrite(&thalf,SFLOAT,1,binLib)*SFLOAT;
  offset += fwrite(E,SFLOAT,3,binLib)*SFLOAT;

  /* write info for each daughter */
  for (unique=0;unique<nRxns;unique++)
    {
      tmpIdx << "\t" << daugKza[unique] << "\t" << emitted[unique] 
	     << "\t" << offset << endl;
      offset+=fwrite(daugKza+unique,SINT,1,binLib)*SINT;
      emittedLen = strlen(emitted[unique]);
      offset+=fwrite(&emittedLen,SINT,1,binLib)*SINT;
      offset+=fwrite(*(emitted+unique),1,emittedLen,binLib);
      offset+=fwrite(*(xSection+unique),SFLOAT,nGroups+1,binLib)*SFLOAT;

      /* delete used info */
      delete emitted[unique];
      delete xSection[unique];
    }

  /* delete used info */
  delete emitted;
  delete xSection;
  delete daugKza;
      
}


void ALARALib::close(int readNParents, int libType, char *idxName)
{

  nParents = readNParents;

  /* close the index */
  tmpIdx.close();

  debug(1,"Current binary location %d vs offset %d",ftell(binLib),offset);
  /* fill the placeholders for the index offset 
   * and number of parents */
  fseek(binLib,0L,SEEK_SET);
  fwrite(&offset,SLONG,1,binLib);
  fwrite(&nParents,SINT,1,binLib);

  verbose(2,"Finished converting to binary with %d parents.",nParents);

  /* append index to file */
  appendIdx(idxName,libType);
  verbose(2,"Appended index to file.");

  /* close library */
  fclose(binLib);

}

/* append the index to end of the binary file */
void ALARALib::appendIdx(char* idxName,int libType)
{
  long offset = 0, ioffset;
  int parNum, nRxn, rxnNum;
  char emission[64];
  int emittedLen, kza;

  /* open text index */
  tmpIdx.open(idxName, ios::in);
  /* go to end of binary library */
  fseek(binLib,0L,SEEK_END);

  debug(2,"Appending index at %d",ftell(binLib));
  /* write the library type, # of Parents and # of Groups */
  offset += fwrite(&libType,SINT,1,binLib)*SINT;
  offset += fwrite(&nParents,SINT,1,binLib)*SINT;
  debug(3,"Wrote number of parents: %d",nParents);
  offset += fwrite(&nGroups,SINT,1,binLib)*SINT;

  /* group boundary and integral flux cards */
  tmpIdx >> kza >> ioffset;
  offset += fwrite(&kza,SINT,1,binLib)*SINT;
  offset += fwrite(&ioffset,SLONG,1,binLib)*SLONG;
  tmpIdx >> kza >> ioffset;
  offset += fwrite(&kza,SINT,1,binLib)*SINT;
  offset += fwrite(&ioffset,SLONG,1,binLib)*SLONG;

  /* for each parent */
  for (parNum=0;parNum<nParents;parNum++)
    {
      /* read and write parent info */
      tmpIdx >> kza >> nRxn >> ioffset;
      offset += fwrite(&kza,SINT,1,binLib)*SINT;
      offset += fwrite(&nRxn,SINT,1,binLib)*SINT;
      offset += fwrite(&ioffset,SLONG,1,binLib)*SLONG;
      
      /* read and write daughter info */
      for (rxnNum=0;rxnNum<nRxn;rxnNum++)
	{
	  tmpIdx >> kza >> emission >> ioffset;
	  emittedLen = strlen(emission);
	  offset += fwrite(&kza,SINT,1,binLib)*SINT;
	  offset += fwrite(&emittedLen,SINT,1,binLib)*SINT;
	  offset += fwrite(emission,1,emittedLen,binLib);
	  offset += fwrite(&ioffset,SLONG,1,binLib)*SLONG;
	}
    } 

  tmpIdx.close();
}

