/* File sections:
 * Service: constructors, destructors
 * Lib: functions directly related to library handling
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "ASCIILib.h"

/****************************
 ********* Service **********
 ***************************/

ASCIILib::ASCIILib(int setType) : DataLib(setType)
{

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
  E[1] = 0;
  E[2] = 0;
  E[3] = 0;
  
  kza = 0;
  nRxns = 0;
  daugKza = NULL;
  mThalf = 0;
  mE[1] = 0;
  mE[2] = 0;
  mE[3] = 0;
  mXsection = NULL;
  mEmitted = NULL;

  binLib = NULL;
}

ASCIILib::~ASCIILib()
{
  delete grpBnds;
  delete grpWeights;

  delete transKza;
  delete [] xSection;
  delete [] emitted;

  delete decayKza;
  delete bRatio;

  fclose(binLib);
  tmpIdx.close();
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

/**************************************
 ********** Write binary data *********
 *************************************/

/* write the data for a single isotope */
void ASCIILib::writeData(long& offset)
{
  int unique, emittedLen;

  /* write parent isotope info */
  tmpIdx << kza << "\t" << nRxns << "\t" << offset << endl;
  offset += fwrite(&kza,SINT,1,binLib)*SINT;
  offset += fwrite(&nRxns,SINT,1,binLib)*SINT;
  offset += fwrite(&mThalf,SFLOAT,1,binLib)*SFLOAT;
  offset += fwrite(mE,SFLOAT,3,binLib)*SFLOAT;

  /* write info for each daughter */
  for (unique=0;unique<nRxns;unique++)
    {
      tmpIdx << "\t" << daugKza[unique] << "\t" << mEmitted[unique] 
	     << "\t" << offset << endl;
      offset+=fwrite(daugKza+unique,SINT,1,binLib)*SINT;
      emittedLen = strlen(mEmitted[unique]);
      offset+=fwrite(&emittedLen,SINT,1,binLib)*SINT;
      offset+=fwrite(*(mEmitted+unique),1,emittedLen,binLib);
      offset+=fwrite(*(mXsection+unique),SFLOAT,nGroups+1,binLib)*SFLOAT;

      /* delete used info */
      delete mEmitted[unique];
      delete mXsection[unique];
    }

  /* delete used info */
  delete mEmitted;
  delete mXsection;
  delete daugKza;
      
}

/* append the index to end of the binary file */
void ASCIILib::appendIdx()
{
  long offset = 0, ioffset;
  int parNum, nRxn, rxnNum;
  char libType = '1', emission[64];
  int emittedLen;

  /* open text index */
  tmpIdx.open(TMPIDXFNAME, ios::in);
  /* go to end of binary library */
  fseek(binLib,0L,SEEK_END);

  debug(2,"Appending index at %d",ftell(binLib));
  /* write the library type, # of Parents and # of Groups */
  offset += fwrite(&libType,1,1,binLib);
  offset += fwrite(&nParents,SINT,1,binLib);
  debug(3,"Wrote number of parents: %d",nParents);
  offset += fwrite(&nGroups,SINT,1,binLib);

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
}

/*****************************************
 ********** Binary Library Mgmt **********
 ****************************************/

void ASCIILib::makeBinLib()
{

  long offset = 0;
  int tKza, dKza;

  tmpIdx.open(TMPIDXFNAME, ios::out);
  binLib = fopen(BINFNAME, "wb");

  /* get initial info from text files */
  getTransInfo();
  debug(4,"Got transmutation header info.");
  getDecayInfo();
  debug(4,"Got decay header info.");

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
	  debug(4,"Got next transmutation entry.");
	  verbose(2,"Wrote transmutation entry for %d (%d)",kza,offset);
	}
      else if (dKza < tKza)
	{
	  /* make a pure decay entry */
	  debug(4,"Writing pure decay entry for %d.",tKza);
	  kza = dKza;
	  decay2merge();
	  dKza = getDecayData();
	  debug(4,"Got next decay entry.");
	  verbose(2,"Wrote decay entry for %d (%d)",kza,offset);
	}
      else
	{
	  /* make a merged entry */
	  debug(4,"Writing merged entry for %d.",tKza);
	  kza = tKza;
	  merge();
	  tKza = getTransData();
	  debug(4,"Got next transmutation entry.");
	  dKza = getDecayData();
	  debug(4,"Got next decay entry.");
	  verbose(2,"Wrote merged entry for %d (%d)",kza,offset);
	}

      /* write the entry to the output files */
      writeData(offset);

      /* increment the number of parents */
      nParents++;
    }

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
  appendIdx();
  verbose(2,"Appended index to file.");

  /* close library */
  fclose(binLib);

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
  error(201, 
	"Programming error: ASCIILib::getTransInfo() must be called from a derived object.");
}

void ASCIILib::getDecayInfo()
{
  error(202,
	"Programming error: ASCIILib::getDecayInfo() must be called from a derived object.");
}


int ASCIILib::getTransData()
{
  error(203,
	"Programming error: ASCIILib::getTransData() must be called from a derived object.");
  return -1;
}

int ASCIILib::getDecayData()
{
  error(204,
	"Programming error: ASCIILib::getDecayData() must be called from a derived object.");
  return -1;
}

void ASCIILib::readData(int getKza, NuclearData* data)
{
  error(205,
	"Programming error: ASCIILib::readData() must be called from a derived object.");

}
