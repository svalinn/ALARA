/* $Id: ASCIILib.C,v 1.7 2001-07-10 20:52:06 wilsonp Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Lib: functions directly related to library handling
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "ASCIILib.h"

#include "DataLib/ALARALib/ALARALib.h"

/****************************
 ********* Service **********
 ***************************/

ASCIILib::ASCIILib(int setType) : DataLib(setType)
{
  for (int idx=0;idx<81;idx++)
    {
      transTitle[idx] = '\0';
      decayTitle[idx] = '\0';
      groupDesc[idx] = '\0';
    }

  grpBnds = NULL;
  grpWeights = NULL;
  
  nTRxns = 0;
  transKza = NULL;
  xSection = NULL;
  emitted = NULL;
  
  nDRxns = 0;
  nIons = 0;
  decayKza = NULL;
  thalf = 0;
  bRatio = NULL;
  E[0] = 0;
  E[1] = 0;
  E[2] = 0;
  
  kza = 0;
  nRxns = 0;
  daugKza = NULL;
  mThalf = 0;
  mE[0] = 0;
  mE[1] = 0;
  mE[2] = 0;
  mXsection = NULL;
  mEmitted = NULL;

  binLib = NULL;
}

ASCIILib::~ASCIILib()
{
  delete grpBnds;
  delete grpWeights;

  delete binLib;
}

/****************************
 *********** Lib ************
 ***************************/

/************************************
 ************* Merge data ***********
 ************************************
 * 
 * Each merge function must set the following variables
 *  - nRxns
 *  - daugKza[]
 *  - mThalf
 *  - mE[3]
 *  - mXsection[][]
 *  - mEmitted[][]
 */

/* convert trans data to merge data */
void ASCIILib::trans2merge()
{
  int rxnNum, unique, gNum;
  char *emittedEnd;

  nRxns = 0;
  daugKza = new int[nTRxns];
  memCheck(daugKza,"ASCIILib::trans2merge(...): daugKza");
  mThalf = 0;
  mE[0] = 0;  mE[1] = 0;  mE[2] = 0;

  mXsection = new float*[nTRxns];
  memCheck(mXsection,"ASCIILib::trans2merge(...): mXsection");

  mEmitted = new char*[nTRxns];
  memCheck(mEmitted,"ASCIILib::trans2merge(...): mEmitted");

  /* for each trans reaction */
  for (rxnNum=0;rxnNum<nTRxns;rxnNum++)
    /* ensure that it is non-zero */
    if (sum(xSection[rxnNum])>0)
      {
	debug(6,"Checking next reaction: %d",
	      transKza[rxnNum]);

	/* truncate the emitted string */
	emittedEnd = strchr(emitted[rxnNum],' ');
	if (emittedEnd != NULL)
	  *emittedEnd = '\0';
	
	/* compare it to each of the previous unique reactions */
	for (unique=0;unique<nRxns;unique++)
	  /* if this reaction product matches a previous product */
	  if (transKza[rxnNum] == daugKza[unique] 
	      && strcmp(emitted[rxnNum],"x"))
	    {
	      /* increment the cross-section */
	      for (gNum=0;gNum<nGroups;gNum++)
		mXsection[unique][gNum] += xSection[rxnNum][gNum];
	      
	      /* concatenate the emitted strings */
	      char *tmp = new char[strlen(mEmitted[unique])+
				  strlen(emitted[rxnNum])+2];
	      memCheck(tmp,"ASCIILib::trans2merge(...): tmp -> mEmitted[n]");
	      sprintf(tmp,"%s,%s",mEmitted[unique],emitted[rxnNum]);
	      delete mEmitted[unique];
	      mEmitted[unique] =tmp;
	      
	      /* end for loop AND use as flag below */
	      unique = nRxns+1;
	    }

	/* if a new reaction product */
	if (unique == nRxns)
	  {
	    debug(6,"Adding unique reaction %d",unique);
	    daugKza[unique] = transKza[rxnNum];
	    debug(7,"Set daughter kza",daugKza[unique]);
	    mXsection[unique] = new float[nGroups+1];
	    memCheck(mXsection[unique],
		     "ASCIILib::trans2merge(...): mXsection[n]");
	    mXsection[unique][nGroups] = 0;
	    for (gNum=0;gNum<nGroups;gNum++)
	      mXsection[unique][gNum] = xSection[rxnNum][gNum];
	    debug(7,"Set xsection");
	    mEmitted[unique] = new char[strlen(emitted[rxnNum])+1];
	    memCheck(mEmitted[unique],
		     "ASCIILib::trans2merge(...): mEmitted[n]");
	    strcpy(mEmitted[unique],emitted[rxnNum]);
	    debug(7,"Set emitted %s",mEmitted[unique]);
	    nRxns++;
	  }
	
      }
}

/* convert decay data to merge data */
void ASCIILib::decay2merge()
{
  int rxnNum, unique, gNum;

  nRxns = 0;
  daugKza = new int[nDRxns];
  memCheck(daugKza,"ASCIILib::decay2merge(...): daugKza");
  mThalf = thalf;
  mE[0] = E[0];  mE[1] = E[1];  mE[2] = E[2];
  mXsection = new float*[nDRxns];
  memCheck(mXsection,"ASCIILib::decay2merge(...): mXsection");
  mEmitted = new char*[nDRxns];
  memCheck(mEmitted,"ASCIILib::decay2merge(...): mEmitted");

  /* for each decay reaction */
  for (rxnNum=0;rxnNum<nDRxns;rxnNum++)
    /* ensure that it is non-zero */
    if (bRatio[rxnNum]>0)
      {
	/* compare it to each of the previous unique reactions */
	for (unique=0;unique<nRxns;unique++)
	  /* if this reaction product matches a previous product */
	  if (daugKza[unique] == decayKza[rxnNum])
	    {
	      /* increment the cross-section */
	      mXsection[unique][nGroups] += bRatio[rxnNum]*log(2.0)/thalf;
	      
	      /* concatenate the emitted strings */
	      char *tmp = new char[strlen(mEmitted[unique])+4];
	      memCheck(tmp,"ASCIILib::decay2merge(...): tmp -> mEmitted[n]");
	      sprintf(tmp,"%s,*%c",mEmitted[unique],
		      (nDRxns-rxnNum>nIons?'D':'X'));
	      delete mEmitted[unique];
	      mEmitted[unique] =tmp;
	      
	      /* end for loop AND use as flag below */
	      unique = nRxns+1;
	    }
	
	/* if a new reaction product */
	if (unique == nRxns)
	  {
	    mXsection[unique] = new float[nGroups+1];
	    memCheck(mXsection[unique],
		     "ASCIILib::decay2merge(...): mXsection[n]");
	    for (gNum=0;gNum<nGroups;gNum++)
	      mXsection[unique][gNum] = 0;
	    daugKza[unique] = decayKza[rxnNum];
	    mXsection[unique][nGroups] = bRatio[rxnNum]*log(2.0)/thalf;
	    mEmitted[unique] = new char[3];
	    memCheck(mEmitted[unique],
		     "ASCIILib::decay2merge(...): mEmitted[n]");
	    strcpy(mEmitted[unique],"*D");
	    nRxns++;
	  }
      }

}

/* merge trans and decay data into a single data block */
void ASCIILib::merge()
{
  int rxnNum, unique, gNum;
  char *emittedEnd;

  nRxns = 0;
  daugKza = new int[nTRxns+nDRxns];
  memCheck(daugKza,"ASCIILib::trans2merge(...): daugKza");
  mThalf = thalf;
  mE[0] = E[0];  mE[1] = E[1];  mE[2] = E[2];
  mXsection = new float*[nTRxns+nDRxns];
  memCheck(mXsection,"ASCIILib::trans2merge(...): mXsection");
  mEmitted = new char*[nTRxns+nDRxns];
  memCheck(mEmitted,"ASCIILib::trans2merge(...): mEmitted");

  /* for each trans reaction */
  for (rxnNum=0;rxnNum<nTRxns;rxnNum++)
    /* ensure that it is non-zero */
    if (sum(xSection[rxnNum])>0)
      {
	/* truncate the emitted string */
	emittedEnd = strchr(emitted[rxnNum],' ');
	if (emittedEnd != NULL)
	  *emittedEnd = '\0';
	
	/* compare it to each of the previous unique reactions */
	for (unique=0;unique<nRxns;unique++)
	  /* if this reaction product matches a previous product */
	  if (transKza[rxnNum] == daugKza[unique] 
	      && strcmp(emitted[rxnNum],"x"))
	    {
	      /* increment the cross-section */
	      for (gNum=0;gNum<nGroups;gNum++)
		mXsection[unique][gNum] += xSection[rxnNum][gNum];
	      
	      /* concatenate the emitted strings */
	      char *tmp = new char[strlen(mEmitted[unique])+
				  strlen(emitted[rxnNum])+2];
	      memCheck(tmp,"ASCIILib::trans2merge(...): tmp -> mEmitted[n]");
	      sprintf(tmp,"%s,%s",mEmitted[unique],emitted[rxnNum]);
	      delete mEmitted[unique];
	      mEmitted[unique] =tmp;
	      
	      /* end for loop AND use as flag below */
	      unique = nRxns+1;
	    }
	
	/* if a new reaction product */
	if (unique == nRxns)
	  {
	    daugKza[unique] = transKza[rxnNum];
	    mXsection[unique] = new float[nGroups+1];
	    memCheck(mXsection[unique],
		     "ASCIILib::trans2merge(...): mXsection[n]");
	    mXsection[unique][nGroups] = 0;
	    for (gNum=0;gNum<nGroups;gNum++)
	      mXsection[unique][gNum] = xSection[rxnNum][gNum];
	    mEmitted[unique] = new char[strlen(emitted[rxnNum])+1];
	    memCheck(mEmitted[unique],
		     "ASCIILib::trans2merge(...): mEmitted[n]");
	    strcpy(mEmitted[unique],emitted[rxnNum]);
	    nRxns++;
	  }
	
      }
  
  /* for each decay reaction */
  for (rxnNum=0;rxnNum<nDRxns;rxnNum++)
    /* ensure that it is non-zero */
    if (bRatio[rxnNum]>0)
      {
	/* compare it to each of the previous unique reactions */
	for (unique=0;unique<nRxns;unique++)
	  /* if this reaction product matches a previous product */
	  if (daugKza[unique] == decayKza[rxnNum])
	    {
	      /* increment the cross-section */
	      mXsection[unique][nGroups] += bRatio[rxnNum]*log(2.0)/thalf;
	      
	      /* concatenate the emitted strings */
	      char *tmp = new char[strlen(mEmitted[unique])+4];
	      memCheck(tmp,"ASCIILib::trans2merge(...): tmp -> mEmitted[n]");
	      sprintf(tmp,"%s,*%c",mEmitted[unique],
		      (nDRxns-rxnNum>nIons?'D':'X'));
	      delete mEmitted[unique];
	      mEmitted[unique] =tmp;
	      
	      /* end for loop AND use as flag below */
	      unique = nRxns+1;
	    }
	
	/* if a new reaction product */
	if (unique == nRxns)
	  {
	    mXsection[unique] = new float[nGroups+1];
	    memCheck(mXsection[unique],
		     "ASCIILib::trans2merge(...): mXsection[n]");
	    for (gNum=0;gNum<nGroups;gNum++)
	      mXsection[unique][gNum] = 0;
	    daugKza[unique] = decayKza[rxnNum];
	    mXsection[unique][nGroups] = bRatio[rxnNum]*log(2.0)/thalf;
	    mEmitted[unique] = new char[3];
	    memCheck(mEmitted[unique],
		     "ASCIILib::trans2merge(...): mEmitted[n]");
	    strcpy(mEmitted[unique],"*D");
	    nRxns++;
	  }
      }
  
}  

/*****************************************
 ********** Binary Library Mgmt **********
 ****************************************/

void ASCIILib::makeBinLib(const char *alaraFname)
{

  int tKza, dKza, nGammaParents=0, writeGamma;
  char alaraIdxName[256], alaraLibName[256];
  char gammaIdxName[256], gammaLibName[256];

  strcpy(alaraIdxName,alaraFname);
  strcat(alaraIdxName,".idx");
  strcpy(alaraLibName,alaraFname);
  strcat(alaraLibName,".lib");  
  binLib = new ALARALib(alaraLibName, alaraIdxName);

  strcpy(gammaIdxName,alaraFname);
  strcat(gammaIdxName,".gdx");
  strcpy(gammaLibName,alaraFname);
  strcat(gammaLibName,".gam");  
  gammaLib = new ALARALib(gammaLibName,gammaIdxName);

  /* get initial info from text files */
  getTransInfo();
  debug(4,"Got transmutation header info.");
  getDecayInfo();
  debug(4,"Got decay header info.");

  binLib->writeHead(nGroups,grpBnds,grpWeights);
  gammaLib->writeHead(0,NULL,NULL);

  /* get first entries from each library */
  tKza = getTransData();
  debug(4,"Got next transmutation entry.");
  dKza = getDecayData();
  debug(4,"Got next decay entry.");
  /* while we are not at the last isotope in both libraries */
  while (tKza != LASTISO || dKza != LASTISO)
    {  
      /* establish order */
      if (tKza < dKza)
	{
	  /* make a pure trans entry */
	  debug(4,"Writing pure transmutation entry for %d.",tKza);
	  kza = tKza;
	  trans2merge();
	  tKza = getTransData();
	  writeGamma = FALSE;
	  debug(4,"Got next transmutation entry.");
	}
      else if (dKza < tKza)
	{
	  /* make a pure decay entry */
	  debug(4,"Writing pure decay entry for %d.",dKza);
	  kza = dKza;
	  decay2merge();
	  if (numSpec > 0)
	    {
	      gammaLib->writeGammaData(kza,numSpec,numDisc,nIntReg,nPnts,
				       discGammaE,discGammaI,intRegB,intRegT,
				       contX,contY);
	      nGammaParents++;
	    }
	  dKza = getDecayData();
	  debug(4,"Got next decay entry.");
	}
      else
	{
	  /* make a merged entry */
	  debug(4,"Writing merged entry for %d.",tKza);
	  kza = tKza;
	  merge();
	  tKza = getTransData();
	  debug(4,"Got next transmutation entry.");
	  if (numSpec > 0)
	    {
	      gammaLib->writeGammaData(kza,numSpec,numDisc,nIntReg,nPnts,
				       discGammaE,discGammaI,intRegB,intRegT,
				       contX,contY);
	      nGammaParents++;
	    }
	  dKza = getDecayData();
	  debug(4,"Got next decay entry.");
	}

      /* write the entry to the output files */
      binLib->writeData(kza,nRxns,mThalf,mE,daugKza,mEmitted,mXsection);

      /* increment the number of parents */
      nParents++;
    }

  binLib->close(nParents,DATALIB_ALARA,alaraIdxName);
  gammaLib->close(nGammaParents,DATALIB_GAMMA,gammaIdxName);

}



/****************************
 ********* Utility **********
 ***************************/

float ASCIILib::sum(float* vector)
{
  int gNum;
  float total = 0;

  for (gNum=0;gNum<nGroups;gNum++)
    total += vector[gNum];

  debug(6,"Cross-section sum: %g",total);
  return total;
}
  


/****************************
 ********* Virtual **********
 ***************************/

void ASCIILib::getTransInfo()
{
  error(9000, 
	"Programming error: ASCIILib::getTransInfo() must be called from a derived object.");
}

void ASCIILib::getDecayInfo()
{
  error(9000,
	"Programming error: ASCIILib::getDecayInfo() must be called from a derived object.");
}


int ASCIILib::getTransData()
{
  error(9000,
	"Programming error: ASCIILib::getTransData() must be called from a derived object.");
  return -1;
}

int ASCIILib::getDecayData()
{
  error(9000,
	"Programming error: ASCIILib::getDecayData() must be called from a derived object.");
  return -1;
}

void ASCIILib::readData(int /*getKza*/, NuclearData* /*data*/)
{
  error(9000,
	"Programming error: ASCIILib::readData() must be called from a derived object.");

}
