/* $Id: ALARALib.C,v 1.14 2003-10-28 22:11:36 wilsonp Exp $ */
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
  if (binLib == NULL)
    error(1105,
	  "The specified library with filename %s could not be created. Please check the path/filename.",
	  fname);
    
  tmpIdx.open(idxName, ios::out);
  offset = 0;
  
  idx = NULL;
}

/* open an existing library */
ALARALib::ALARALib(char* fname,int setType) 
  : DataLib(setType)
{

  char fnameStr[256];
  strcpy(fnameStr,fname);
  strcat(fnameStr,libTypeSuffix[type]);

  binLib = fopen(searchXSPath(fnameStr),"rb");
  if (binLib == NULL)
    error(1104,
	  "The specified library with filename %s could not be accessed. Please check the path/filename.",
	  fnameStr);
    

  idx = new LibIdx(nParents,nGroups,binLib,setType);

  if (setType != type)
    error(1100,"You have specified library type %s but given the filename of a%s library.",
	  libTypeStr[type],libTypeStr[setType]);
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
  int checkKza, nRxns=0;
  float thalf = 0, E[3] = {0,0,0};
  int *daugKza = NULL;
  char **emitted = NULL;
  float **xSection = NULL, *totalXSect=NULL;
  int rxnNum, emittedLen, numNZGrps, gNum;

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
      if (type == DATALIB_ADJOINT)
	{
	  totalXSect = new float[nGroups+1];
	  fread(totalXSect,SFLOAT,nGroups+1,binLib);
	}

      /* setup arrays */
      daugKza= new int[nRxns];
      memCheck(daugKza,"ALARALib::readData(...): daugKza");

      xSection = new float*[nRxns];
      memCheck(xSection,"ALARALib::readData(...): xSection");

      emitted = new char*[nRxns];
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
	  fread(&numNZGrps,SINT,1,binLib);
	  fread(xSection[rxnNum],SFLOAT,numNZGrps,binLib);
	  for (gNum=numNZGrps;gNum<nGroups;gNum++)
	    xSection[rxnNum][gNum] = 0;
	  fread(xSection[rxnNum]+nGroups,SFLOAT,1,binLib);
	}
      
      verbose(5,"Read %d reaction path(s) for %d.",nRxns,findKza);
      /* set the NuclearData object */
    }
  data->setData(nRxns,E,daugKza,emitted,xSection,thalf,totalXSect);
      
  for (rxnNum=0;rxnNum<nRxns;rxnNum++)
    {
      delete xSection[rxnNum];
      delete emitted[rxnNum];
    }
  delete xSection;
  delete emitted;
  delete daugKza;
  delete totalXSect;

  xSection = NULL;
  emitted = NULL;
  daugKza = NULL;
  totalXSect = NULL;
  
}

/* read gamma data for one isotope */
void ALARALib::readGammaData(int findKza, GammaSrc *gammaSrc)
{

  long offset;
  int checkKza, specNum, numSpec=0;
  int *numDisc=NULL, *numIntReg=NULL, *nPnts=NULL;
  int **intRegB=NULL, **intRegT=NULL;
  float **discGammaE=NULL, **discGammaI=NULL, **contX=NULL, **contY=NULL;

  /* search index and go to that location */
  offset = idx->search(findKza);


  if (offset > 0)
    {
      fseek(binLib,offset,SEEK_SET);
      
      fread(&checkKza,SINT,1,binLib);
      fread(&numSpec,SINT,1,binLib);

      numDisc = new int[numSpec];
      discGammaE = new float*[numSpec];
      discGammaI = new float*[numSpec];

      numIntReg = new int[numSpec];
      nPnts = new int[numSpec];
      intRegB = new int*[numSpec];
      intRegT = new int*[numSpec];
      contX = new float*[numSpec];
      contY = new float*[numSpec];

      fread(numDisc,SINT,numSpec,binLib);
      fread(numIntReg,SINT,numSpec,binLib);
      fread(nPnts,SINT,numSpec,binLib);
      
      for (specNum=0;specNum<numSpec;specNum++)
	{
	  discGammaE[specNum] = new float[numDisc[specNum]];
	  discGammaI[specNum] = new float[numDisc[specNum]];
	  fread(discGammaE[specNum],SFLOAT,numDisc[specNum],binLib);
	  fread(discGammaI[specNum],SFLOAT,numDisc[specNum],binLib);

	  intRegB[specNum] = new int[numIntReg[specNum]];
	  intRegT[specNum] = new int[numIntReg[specNum]];
	  contX[specNum] = new float[nPnts[specNum]];
	  contY[specNum] = new float[nPnts[specNum]];
	  fread(intRegB[specNum],SINT,numIntReg[specNum],binLib);
	  fread(intRegT[specNum],SINT,numIntReg[specNum],binLib);
	  fread(contX[specNum],SFLOAT,nPnts[specNum],binLib);
	  fread(contY[specNum],SFLOAT,nPnts[specNum],binLib);
	}
    }

  gammaSrc->setData(findKza,numSpec,numDisc,numIntReg,nPnts,intRegB,intRegT,
		    discGammaE,discGammaI,contX,contY);

  for (specNum=0;specNum<numSpec;specNum++)
    {
      delete intRegB[specNum];
      delete intRegT[specNum];
      delete discGammaE[specNum];
      delete discGammaI[specNum];
      delete contX[specNum];
      delete contY[specNum];
    }

  delete numDisc;
  delete numIntReg;
  delete nPnts;
  delete intRegB;
  delete intRegT;
  delete discGammaE;
  delete discGammaI;
  delete contX;
  delete contY;

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
  int unique, emittedLen, numNZGrps, gNum;

  verbose(2,"Writing entry for %d (%d)",kza,offset);

  /* write parent isotope info */
  tmpIdx << kza << "\t" << nRxns << "\t" << thalf << "\t" << offset << endl;
  offset += fwrite(&kza,SINT,1,binLib)*SINT;
  offset += fwrite(&nRxns,SINT,1,binLib)*SINT;
  offset += fwrite(&thalf,SFLOAT,1,binLib)*SFLOAT;
  offset += fwrite(E,SFLOAT,3,binLib)*SFLOAT;

  /* write info for each daughter */
  for (unique=0;unique<nRxns;unique++)
    {
      /* determine number of non-zero groups */
      numNZGrps = nGroups;
      while (numNZGrps > 1 && xSection[unique][numNZGrps-1] == 0)
	numNZGrps--;

      tmpIdx << "\t" << daugKza[unique] << "\t" << emitted[unique] 
	     << "\t" << offset << endl;
      offset+=fwrite(daugKza+unique,SINT,1,binLib)*SINT;
      emittedLen = strlen(emitted[unique]);
      offset+=fwrite(&emittedLen,SINT,1,binLib)*SINT;
      offset+=fwrite(*(emitted+unique),1,emittedLen,binLib);
      offset+=fwrite(&numNZGrps,SINT,1,binLib)*SINT;
      offset+=fwrite(*(xSection+unique),SFLOAT,numNZGrps,binLib)*SFLOAT;
      offset+=fwrite((*(xSection+unique))+nGroups,SFLOAT,1,binLib)*SFLOAT;

      /* delete used info */
      delete emitted[unique];
      delete xSection[unique];
    }

  /* delete used info */
  delete emitted;
  delete xSection;
  delete daugKza;
      
}

void ALARALib::writeGammaData(int kza, int numSpec, int *numDisc, int *nIntReg,
			      int *nPnts, float **discGammaE, float **discGammaI,
			      int **intRegB, int **intRegT, 
			      float **contX, float **contY)
{
  int specNum;

  verbose(2,"Writing GAMMA entry for %d (%d)",kza,offset);
  
  /* write data to files */
  tmpIdx << kza << "\t" << numSpec << "\t" << 0 << "\t" << offset << endl;
  
  /* write basic info to binLib */
  offset += fwrite(&kza,SINT,1,binLib)*SINT;
  offset += fwrite(&numSpec,SINT,1,binLib)*SINT;
  /* write number of discrete gammas in each spectrum */
  offset+=fwrite(numDisc,SINT,numSpec,binLib)*SINT;	
  /* write number of interpolation groups in each spectrum */
  offset+=fwrite(nIntReg,SINT,numSpec,binLib)*SINT;
  /* write number of interpolation points in each spectrum */
  offset+=fwrite(nPnts,SINT,numSpec,binLib)*SINT;
  
  /* for each spectrum */
  for (specNum=0;specNum<numSpec;specNum++)
    {
      /* write extra info to index */
      tmpIdx << "\t" << numDisc[specNum] << "\t" << nIntReg[specNum]
	     << "\t" << nPnts[specNum] << "\t" << offset << endl;
      
      /* gamma Energies */
      offset+=fwrite(discGammaE[specNum],SFLOAT,numDisc[specNum],binLib)*SFLOAT;
      delete discGammaE[specNum];
      /* gamma Intensities */
      offset+=fwrite(discGammaI[specNum],SFLOAT,numDisc[specNum],binLib)*SFLOAT;
      delete discGammaI[specNum];
      /* interpolation region boundaries */
      offset+=fwrite(intRegB[specNum],SINT,nIntReg[specNum],binLib)*SINT;
      delete intRegB[specNum];
      /* interpolation region types */
      offset+=fwrite(intRegT[specNum],SINT,nIntReg[specNum],binLib)*SINT;
      delete intRegT[specNum];
      /* interpolation gamma Energies */
      offset+=fwrite(contX[specNum],SFLOAT,nPnts[specNum],binLib)*SFLOAT;
      delete contX[specNum];
      /* interpolation gamma Intensities */
      offset+=fwrite(contY[specNum],SFLOAT,nPnts[specNum],binLib)*SFLOAT;
      delete contY[specNum];
    }
  
  delete numDisc;
  delete nIntReg;
  delete nPnts;
  delete discGammaE;
  delete discGammaI;
  delete intRegB;
  delete intRegT;
  delete contX;
  delete contY;
  
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
  long ioffset;
  int parNum, nRxn, rxnNum;
  float thalf;
  char emission[64];
  int emittedLen, kza;
  int nDisc, nRegs, nPnts;

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
      tmpIdx >> kza >> nRxn >> thalf >> ioffset;
      offset += fwrite(&kza,SINT,1,binLib)*SINT;
      offset += fwrite(&nRxn,SINT,1,binLib)*SINT;
      offset += fwrite(&ioffset,SLONG,1,binLib)*SLONG;
      
      /* read and write daughter info */
      for (rxnNum=0;rxnNum<nRxn;rxnNum++)
	{
	  switch(libType)
	    {
	    case DATALIB_ALARA:
	    case DATALIB_ADJOINT:
	      {
		tmpIdx >> kza >> emission >> ioffset;
		emittedLen = strlen(emission);
		offset += fwrite(&kza,SINT,1,binLib)*SINT;
		offset += fwrite(&emittedLen,SINT,1,binLib)*SINT;
		offset += fwrite(emission,1,emittedLen,binLib);
		offset += fwrite(&ioffset,SLONG,1,binLib)*SLONG;
		break;
	      }
	    case DATALIB_GAMMA:
	      {
		tmpIdx >> nDisc >> nRegs >> nPnts >> ioffset;
		offset += fwrite(&nDisc,SINT,1,binLib)*SINT;
		offset += fwrite(&nRegs,SINT,1,binLib)*SINT;
		offset += fwrite(&nPnts,SINT,1,binLib)*SINT;
		offset += fwrite(&ioffset,SLONG,1,binLib)*SLONG;
	      }
	    }
	}
    } 

  tmpIdx.close();
}

