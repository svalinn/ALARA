/* $Id: EAFLib.C,v 1.12 2003-10-28 22:11:36 wilsonp Exp $ */
#include "EAFLib.h"
#include "DataLib/ALARALib_def.h"

EAFLib::EAFLib(const char *transFname, const char *decayFname, const char *alaraFname) 
  : ASCIILib(DATALIB_EAF)
{
  if (transFname != NULL && decayFname != NULL)
    {
      char* transFilePath = searchNonXSPath(transFname);
      char* decayFilePath = searchNonXSPath(decayFname);
      inTrans.open(transFilePath, ios::in);
      inDecay.open(decayFilePath, ios::in);
      free(transFilePath);
      free(decayFilePath);

      makeBinLib(alaraFname);
    }

}

// Decay-only constructor (for use in ALARAJOY)
EAFLib::EAFLib(const char* decayFname) : ASCIILib(DATALIB_EAF)
{
  // Only open the decay stream -- do not call makeBinLib here
  if (decayFname !=NULL)
  {
    inDecay.open(searchNonXSPath(decayFname), ios::in);
    if (!inDecay.is_open())
      error(1002, "EAFLib(decay-only): failed to open decay file %s", decayFname);
  }
}

EAFLib::~EAFLib()
{
  /* delete arrays here that were dimensioned with
   * EAF relevant constants */
  int rxnNum;

  for (rxnNum=0;rxnNum<MAXEAFRXNS;rxnNum++)
    {
      delete[] xSection[rxnNum];
      delete[] emitted[rxnNum];
    }
  delete[] xSection;
  delete[] emitted;
  delete[] transKza;

  delete[] decayKza;
  delete[] bRatio;

  inTrans.close();
  inDecay.close();
}

void EAFLib::extract(char* input, float* value)
{
  char section[32];

  /* find beginning of exponent */
  int expStart = strcspn(input,"+-");

  /* extract mantissa portion to string variable */
  strncpy(section,input,expStart);
  section[expStart] = '\0';

  /* read the mantissa of the number */
  float Man = atof(section);

  /* extract exponent portion to string variable */
  strncpy(section,input+expStart,3);
  section[3] = '\0';

  /* read the exponent */
  int Exp = atoi(section);

  /* return the value */
  *value = Man*pow(10,Exp);
  return;
}

int EAFLib::delKza(float rxnType)
{
  int baseType = int(rxnType);
  int otherType = int((rxnType-baseType)*10);

  int result;

  switch (baseType)
    {
    case 1:
      result = -1000;
      break;
    case 2:
      result = 1000;
      break;
    case 3:
      result = 0;
      break;
    case 4:
      result = 2004;
      break;
    case 5:
      result = 1;
      break;
    case 7:
      result = 1001;
      break;
    }

  switch (otherType)
    {
    case 1:
      result += -1000;
      break;
    case 2:
      result += 1000;
      break;
    case 3:
      result += 0;
      break;
    case 4:
      result += 2004;
      break;
    case 5:
      result += 1;
      break;
    case 7:
      result += 1001;
      break;
    }

  return result;
}

/************************************
 ******* Trans data handling ********
 ***********************************/

/* read the head of the file and get the following info:
 *  - Title
 *  - nGroups
 *  - group bound text description
 *  - group boundaries
 *  - group weighting
 * and then advance to first isotope */
void EAFLib::getTransInfo()
{
  int strPos, rxnNum;
  char buffer[MAXLINELENGTH];
  
  /* find title line */
  inTrans.getline(buffer,MAXLINELENGTH);
  inTrans.getline(transTitle,MAXLINELENGTH);

  /* set correct length of title card for index file */
  if (strlen(transTitle)<76)
    for (strPos=strlen(transTitle);strPos<76;strPos++)
      transTitle[strPos] = ' ';
  /* truncate to make room for number of parents */
  transTitle[76] = '\0';
  /* parse number of groups from title card */
  nGroups = atoi(strstr(transTitle,"Gp")-3);

  grpBnds = NULL;
  grpWeights = NULL;
  /* looking for last comment card */
  inTrans.getline(buffer,MAXLINELENGTH);
  while (buffer[0] != '#')
    inTrans.getline(buffer,MAXLINELENGTH);
  
  /* initialize arrays */
  transKza = new int[MAXEAFRXNS];
  memCheck(transKza,"EAFLib::getTransInfo(...): transKza");

  xSection = new float*[MAXEAFRXNS];
  memCheck(xSection,"EAFLib::getTransInfo(...): xSection");

  emitted = new char*[MAXEAFRXNS];
  memCheck(emitted,"EAFLib::getTransInfo(...): emitted");

  for (rxnNum=0;rxnNum<MAXEAFRXNS;rxnNum++)
    {
      xSection[rxnNum] = new float[nGroups+1];
      memCheck(xSection[rxnNum],"EAFLib::getTransInfo(...): xSection[n]");

      xSection[rxnNum][nGroups] = 0;

      emitted[rxnNum] = new char[6];
      memCheck(emitted[rxnNum],"EAFLib::getTransInfo(...): emitted[n]");
    }
  
}


void EAFLib::readReaction(int& rxnNum, int zak )
{
  
  /* for forming kza of daughter isotope */
  char daugIsoF,daugIsoC,daugSym[5]=" ";
  int daugIsoN,daugNum;

  char buffer[MAXLINELENGTH];

  int nEntries,gNum;

  /* increment reaction counter */
  rxnNum++;
  
  /* read a line from input file */
  inTrans.getline(buffer,MAXLINELENGTH);
  
  /* parse input */
  /* [reaction code] [x-section entries] 
   * [symbol][mass #][Isomeric Flag][Isomer #](N,[emitted particles])
   * [daughter symbol][duaghter mass#][daughter Iso. Flag][daughter Iso. #] */
  sscanf(buffer,"%*d %d %*2s %*d%*c%*c(N,%3s )%2s %d%c%c",
	 &nEntries,emitted[rxnNum],daugSym+1,&daugNum,&daugIsoF,
	 &daugIsoC);
  
  debug(6,"Found reaction with %d entries, %d(n,%s)%s %d ,%c,%c.",nEntries,zak,emitted[rxnNum],daugSym+1,daugNum,daugIsoF,daugIsoC);
  
  if (daugIsoF == 'M')
    strcat(emitted[rxnNum],"*");
  
  /* expand to correct length: daugSym[4] */
  if (daugSym[2] == '\0')
    {
      daugSym[2] = ' ';
    }
  daugSym[3] = ' ';
  daugSym[4] = '\0';
  
  /* clear two comment lines */
  inTrans.getline(buffer,MAXLINELENGTH);
  inTrans.getline(buffer,MAXLINELENGTH);
  
  debug(6,"Reading xsection %d for %d.",rxnNum,zak);
  
  /* read x-section entries */
  for (gNum=0;gNum<nEntries;gNum++)
    inTrans >> xSection[rxnNum][gNum];
  
  /* set all non-entry groups */
  for (;gNum<nGroups;gNum++)
    xSection[rxnNum][gNum] = xSection[rxnNum][nEntries-1];
  
  /* parse daughter */
  if (daugIsoF == 'M')
    daugIsoN = daugIsoC - '0';
  else
    daugIsoN = 0;
  daugSym[1] = tolower(daugSym[1]);
  daugSym[2] = tolower(daugSym[2]);
  int Z = (strstr(SYMBOLS,daugSym)-SYMBOLS)/3 + 1;
  transKza[rxnNum] = (Z*1000+daugNum)*10 + daugIsoN;

}


/* read a block of data for the next isotope
 * return data:
 *  - kza
 *  - nTRxns
 *  - xSection[]
 *  - transKza[]
 *  - emitted[]
 */
int EAFLib::getTransData()
{

  static int zak = 0;

  /* parent isotope specifics parameters */
  int rxnNum;
  int oldZak;

  /* counters */
  int gNum;
  unsigned int eNum;

  /* for tracking gas production */
  const int numGases = 5;
  int mult,gasNum,gasFlag[numGases];
  int gasKza[numGases]={10030,10020,10010,20040,20030};
  const char *gasList = "tdpah";
  const char *gasEmitted = "x    ";
  static float *gas[numGases]={NULL,NULL,NULL,NULL,NULL};

  if (gas[0] == NULL)
      for (gasNum=0;gasNum<numGases;gasNum++)
	{
	  gas[gasNum] = new float[nGroups];
	  memCheck(gas[gasNum],"EAFLib::getTransData(...): gas[n]");
	}

  /* first time into this routine */
  if (zak == 0)
    /* read modified kza number */
    inTrans >> zak;
  else if (zak == -1)
    {
      debug(5,"End of transmutation file.");
      return LASTISO;
    }

  debug(5,"Found isotope with zak: %d",zak);

  /* used to make sure we are still 
   * reading reactions for the 
   * current isotope */
  oldZak = zak;

  /* initialize reaction counter */
  rxnNum=-1;
  
  for (gasNum=0;gasNum<numGases;gasNum++)
    {
      /* initialize the gas production flags */
      gasFlag[gasNum] = 0;
      
      /* initialize gas production xsections */
      for (gNum=0;gNum<nGroups;gNum++)
	gas[gasNum][gNum] = 0;
    }
  
  debug(6,"Reading through reaction paths for %d",zak);
  /* while this is still the same isotope */
  while (zak==oldZak)
    {
      /* read everything we need to know from the input file */
      readReaction(rxnNum, zak);

      
      /* parse emitted particles */
      /* create a total production cross-section */
      for (eNum=0;eNum<strlen(emitted[rxnNum]);eNum++)
	{
	  emitted[rxnNum][eNum] = tolower(emitted[rxnNum][eNum]);
	  mult=1;
	  /* if this is not the first character AND
	   * the previous character is a digit */
	  if (eNum>0 && isdigit(emitted[rxnNum][eNum-1]))
	    /* determine how many are produced */
	    mult=atoi(emitted[rxnNum]+eNum-1);
	  /* based on which character this is */
	  gasNum = strchr(gasList,emitted[rxnNum][eNum])-gasList;
	  if (gasNum>=0)
	    {
	      gasFlag[gasNum] = 1;
	      for (gNum=0;gNum<nGroups;gNum++)
		gas[gasNum][gNum] += mult*xSection[rxnNum][gNum];
	    }
	}

      inTrans >> zak;

      /* get next modified kza number */
      if (inTrans.eof())
	zak = -1;
      
      /* if fission x-section, dismiss */
      if (tolower(emitted[rxnNum][0]) == 'f')
	{
	  rxnNum--;
	  continue;
	}
    }
  
  /* put all data into correct variables and arrays */
  nTRxns = rxnNum+1;
  
  for (gasNum=0;gasNum<numGases;gasNum++)
    if (gasFlag[gasNum])
      {
	for(gNum=0;gNum<nGroups;gNum++)
	  xSection[nTRxns][gNum] = gas[gasNum][gNum];
	transKza[nTRxns] = gasKza[gasNum];
	strcpy(emitted[nTRxns],gasEmitted);
	nTRxns++;
      }
  
  debug(5,"Found %d reaction paths for %d.",nTRxns,oldZak);
  
  return oldZak;
}

/************************************
 ******* Decay data handling ********
 ***********************************/

/* read the head of the file and get the following info:
 *  - Title
 * and then advance to first isotope */
void EAFLib::getDecayInfo()
{

  char buffer[MAXLINELENGTH];
  int junkInt,i;

  /* read number of initial comment lines */
  inDecay >> junkInt;

  /* read end of this line and comment lines */
  for (i=0;i<=junkInt;i++)
    inDecay.getline(buffer,MAXLINELENGTH);

  decayKza = new int[MAXEAFDCYMODES];
  memCheck(decayKza,"EAFLib::getDecayInfo(...): decayKza");

  bRatio = new float[MAXEAFDCYMODES];
  memCheck(bRatio,"EAFLib::getDecayInfo(...): bRatio");

}

/* read a block of data for the next isotope
 * return data:
 *  - kza
 *  - nDRxns
 *  - nIons
 *  - thalf
 *  - decayKza[]
 *  - E[]
 *  - bratio[]
 */
int EAFLib::getDecayData()
{
  char buffer[MAXLINELENGTH];
  int MT, isoFlag, zak;
  float inFlt, rxnType;
  int pFlag, aFlag, decyNum, numDecayEnergies;
  float pBranch, aBranch, dIsoFlag;
  

  do
    {
      /* scan for first 457 card */
      inDecay.getline(buffer,MAXLINELENGTH);
      /* extract the MT number of this line */
      buffer[75] = '\0';
      MT = atoi(buffer+72);
    } while (MT != 457 && !inDecay.eof());

  if (inDecay.eof())
    {
      debug(5,"End of Decay file.");
      return LASTISO;
    }


  /* extract the number of radiation spectra */
  buffer[66] = '\0';
  numSpec = atoi(buffer+55);
  /* extract the za number of the isotope */
  buffer[44] = '\0';
  isoFlag = atoi(buffer+33);
  extract(buffer,&inFlt);
  zak = int(inFlt+0.1)*10 + isoFlag;

  debug(5,"Found decay data for %d.",zak);
  /* read line */
  /* thalf */
  inDecay.getline(buffer,MAXLINELENGTH);
  /* extract data */
  extract(buffer,&thalf);
  numDecayEnergies=atoi(buffer+44)/2;

  /* read line */
  /* betaA --> gammaE --> alphaE */
  inDecay.getline(buffer,MAXLINELENGTH);
  /* extract data */
  extract(buffer,E);
  extract(buffer+22,E+1);
  extract(buffer+44,E+2);
  if (numDecayEnergies > 3)
    {
      inDecay.getline(buffer,MAXLINELENGTH);
      inDecay.getline(buffer,MAXLINELENGTH);
      inDecay.getline(buffer,MAXLINELENGTH);
      inDecay.getline(buffer,MAXLINELENGTH);
      inDecay.getline(buffer,MAXLINELENGTH);
    }
  
  /* read line */
  /* --> numDecyRxn */
  inDecay.getline(buffer,MAXLINELENGTH);
  /* extract data */
  buffer[66] = '\0';
  nDRxns = atoi(buffer+55);
  
  debug(6,"Found %d decay branches for %d.",nDRxns,zak);

  pFlag=0;
  aFlag=0;
  aBranch=0;
  pBranch=0;
  /* for each decay mode */
  for (decyNum=0;decyNum<nDRxns;decyNum++)
    {
      /* read line */
      /* rxnType --> daughter's isomeric state --> branching ratio */
      inDecay.getline(buffer,MAXLINELENGTH);
      /* extract data */
      extract(buffer,&rxnType);
      extract(buffer+11,&dIsoFlag);
      extract(buffer+44,bRatio+decyNum);
      
      /* set daughter kza based on reaction type*/
      decayKza[decyNum] = (zak/10 - delKza(rxnType))*10
	+ int(dIsoFlag);
      /* if this is a proton producing reaction */
      if (int(rxnType) == 7)
	{
	  pFlag = 1;
	  pBranch += bRatio[decyNum];
	}
      /* if this is an alpha producing reaction */
      else if (int(rxnType) == 4 || int(rxnType*10)%10 == 4)
	{
	  aFlag = 1;
	  aBranch += bRatio[decyNum];
	}
      /* spontaneous fission */
      else if (int(rxnType) == 6)
	{
	  decyNum--;
	  nDRxns--;
	}
      
    }

  /* put all data in correct variables and arrays */
  /* write alpha and proton production rates */
  nIons = 0;
  if (pFlag)
    {
      decayKza[nDRxns] = 10010;
      bRatio[nDRxns] = pBranch;
      nDRxns++;
      nIons++;
    }
  
  if (aFlag)
    {
      decayKza[nDRxns] = 20040;
      bRatio[nDRxns] = aBranch;
      nDRxns++;
      nIons++;
    }
  if (numSpec != 0)
    numSpec = getGammaData();

  /* skip to end of this file section */
  do
    {
      /* scan for last 457 card */
      inDecay.getline(buffer,MAXLINELENGTH);
      /* extract the MT number of this line */
      buffer[75] = '\0';
      MT = atoi(buffer+72);
    } while (MT == 457 && !inDecay.eof());


  debug(6,"Returning %d decay branches for %d.",nDRxns,zak);

  return zak;
}

/************************************
 ******* Gamma data handling ********
 ***********************************/


void EAFLib::skipDiscreteGammas(char* buffer, int numGammas)
{
  int gammaNum, numParms;

  /* for each discrete gamma */
  for (gammaNum=0;gammaNum<numGammas;gammaNum++)
    {
      /* read first line */
      inDecay.getline(buffer,MAXLINELENGTH);
      /* get number of parameters */
      buffer[55] = '\0';
      numParms = atoi(buffer+44);
      /* skip next line */
      inDecay.getline(buffer,MAXLINELENGTH);
      /* if necessary */
      if (numParms>6)
	/* skip another line */
	inDecay.getline(buffer,MAXLINELENGTH);
    }
}

void EAFLib::skipContGammas(char *buffer)
{
  int numPnts, numReg, covFlag;
  int regNum,pntNum;

  /* skip line */
  inDecay.getline(buffer,MAXLINELENGTH);
  /* read covariance flag --> 
   * number of interpolation regions --> 
   * number of interpolations points */
  buffer[66] = '\0';
  numPnts = atoi(buffer+55);
  buffer[55] = '\0';
  numReg = atoi(buffer+44);
  buffer[44] = '\0';
  covFlag = atoi(buffer+33);
  /* skip one line for every three interpolation regions */
  for (regNum=0;regNum<numReg;regNum+=3)
    inDecay.getline(buffer,MAXLINELENGTH);
  /* skip one line for every three data points */
  for (pntNum=0;pntNum<numPnts;pntNum+=3)
    inDecay.getline(buffer,MAXLINELENGTH);
  
  if (covFlag>0)
    {
      inDecay.getline(buffer,MAXLINELENGTH);
      /* read number of points in table */
      buffer[66] = '\0';
      numPnts = atoi(buffer+55);
      /* skip one line for every three data points */
      for (pntNum=0;pntNum<numPnts;pntNum+=3)
	inDecay.getline(buffer,MAXLINELENGTH);
    }
}

void EAFLib::readDiscreteGammas(int specNum, int numGammas, 
				float discFactor, char *buffer ) 
{ 
  int gammaNum; 
  int numParms; 
  float gammaE, gammaI;

  numDisc[specNum] = numGammas;
  discGammaE[specNum] = new float[numGammas];
  discGammaI[specNum] = new float[numGammas];
  
  /* read new list of gamma energy and intensities */
  for (gammaNum=0;gammaNum<numGammas;gammaNum++)
    {
      /* extract gamma energy */
      inDecay.getline(buffer,MAXLINELENGTH);
      extract(buffer,&gammaE);
      buffer[55] = '\0';
      numParms = atoi(buffer+44);
      
      /* extract gamma intensity */
      inDecay.getline(buffer,MAXLINELENGTH);
      extract(buffer+22,&gammaI);
      
      /* throw away extra line if necessary */
      if (numParms>6)
	inDecay.getline(buffer,MAXLINELENGTH);
      
      discGammaE[specNum][gammaNum] = gammaE;
      discGammaI[specNum][gammaNum] = gammaI*discFactor;
    }
}

void EAFLib::readContGammas(int specNum, float contFactor, char* buffer)
{
  int regNum, pntNum;
  int numPnts, numReg, covFlag;
  int b[3],t[3];
  float x, y;

  /* read covariance flag... */
  inDecay.getline(buffer,MAXLINELENGTH);
  /* read covariance flag --> 
   * number of interpolation regions --> 
   * number of interpolations points */
  buffer[66] = '\0';
  numPnts = atoi(buffer+55);
  buffer[55] = '\0';
  numReg = atoi(buffer+44);
  buffer[44] = '\0';
  covFlag = atoi(buffer+33);
  
  /* read interpolation regions */
  nIntReg[specNum] = numReg;
  intRegB[specNum] = new int[numReg];
  intRegT[specNum] = new int[numReg];
  
  /* read each interpolation region */
  for (regNum=0;regNum<numReg;)
    {
      /* record first regNum for this input line */
      int lineStart = regNum;
      /* read this line */
      inDecay.getline(buffer,MAXLINELENGTH);
      /* extract region boundaries and interpolation types */
      buffer[66] = '\0';
      sscanf(buffer,"%d %d %d %d %d %d",b,t,b+1,t+1,b+2,t+2);
      /* for each region on this line */
      for (;regNum<lineStart+3 && regNum<numReg;regNum++)
	{
	  /* set the data */
	  intRegB[specNum][regNum] = b[regNum-lineStart];
	  intRegT[specNum][regNum] = t[regNum-lineStart];
	}
    }
  
  /* read the interpolation table */
  nPnts[specNum] = numPnts;
  contX[specNum] = new float[numPnts];
  contY[specNum] = new float[numPnts];
  
  /* read each interpolation point */
  for (pntNum=0;pntNum<numPnts;)
    {
      /* record first pntNum for this input line */
      int lineStart = pntNum;
      /* read this line */
      inDecay.getline(buffer,MAXLINELENGTH);
      buffer[66] = '\0';
      /* extract spectrum table data */
      /* for each point on this line */
      for (;pntNum<lineStart+3 && pntNum<numPnts;pntNum++)
	{
	  extract(buffer+22*(pntNum-lineStart),&x);
	  extract(buffer+11+22*(pntNum-lineStart),&y);
	  /* set the data */
	  contX[specNum][pntNum] = x;
	  contY[specNum][pntNum] = y*contFactor;
	}
    }
  
  /* skip covariance data */
  if (covFlag>0)
    {
      inDecay.getline(buffer,MAXLINELENGTH);
      /* read number of points in table */
      buffer[66] = '\0';
      numPnts = atoi(buffer+55);
      /* skip one line for every three data points */
      for (pntNum=0;pntNum<numPnts;pntNum+=3)
	inDecay.getline(buffer,MAXLINELENGTH);
    }
}

int EAFLib::getGammaData()
{
  
  char buffer[MAXLINELENGTH];
  int specNum, numGammas, specType;
  float radType, discFactor, contFactor;
  int numGSpec = 0;

  nIntReg = new int[numSpec];
  intRegB = new int*[numSpec];
  intRegT = new int*[numSpec];
  nPnts = new int[numSpec];
  contX = new float*[numSpec];
  contY = new float*[numSpec];
  numDisc = new int[numSpec];
  discGammaE = new float*[numSpec];
  discGammaI = new float*[numSpec];
  
  /* for each spectrum */
  for (specNum=0;specNum<numSpec;specNum++)
    {
      /* read line */
      /* --> decay type, data type --> number of discrete values */
      inDecay.getline(buffer,MAXLINELENGTH);
      /* extract data */
      buffer[66] = '\0';
      /* get the spectrum type */
      extract(buffer+11,&radType);
      
      /* read spectrum type and number of discrete spectra */
      buffer[66] = '\0';
      numGammas = atoi(buffer+55);
      buffer[33] = '\0';
      specType = atoi(buffer+22);
      
      /* read normalization constants */
      inDecay.getline(buffer,MAXLINELENGTH);
      buffer[66] = '\0';
      extract(buffer,&discFactor);
      extract(buffer+44,&contFactor);
      
      /* it is possible that the ENDF entry is all zeros, 
       * in which case, skip this spectrum */
      if ( (discFactor == 0.0 || numGammas == 0) && (specType == 0.0 || contFactor == 0.0) )
	radType = 1;
      
      /* if this is not a gamma spectrum or an x-ray spectrum entry */
      if ( (int(radType) != 0) && (int(radType) != 9) )
	{
	  skipDiscreteGammas(buffer,numGammas);
	  if (specType>0)
	    skipContGammas(buffer);
	}
      /* this is a gamma spectrum entry */
      else
	{
	  /* read this spectrum */
	  /* read discrete entries */
	  numDisc[numGSpec] = 0;
	  discGammaE[numGSpec] = NULL;
	  discGammaI[numGSpec] = NULL;
	  
	  if (numGammas > 0 )
	    {
	    /* if there are valid discrete gammas */
	    if (discFactor > 0 )
	      readDiscreteGammas(numGSpec,numGammas,discFactor,buffer);
	    else
	      skipDiscreteGammas(buffer,numGammas);
	    }
	  
	  nIntReg[numGSpec] = 0;
	  intRegB[numGSpec] = NULL;
	  intRegT[numGSpec] = NULL;
	  nPnts[numGSpec] = 0;
	  contX[numGSpec] = NULL;
	  contY[numGSpec] = NULL;
	  
	  /* read continuous data */
	  if (specType>0)
	    {
	    /* if the scaling factor is non-zero */
	    if (contFactor>0)
	      readContGammas(numGSpec,contFactor,buffer);
	    else
	      skipContGammas(buffer);
	    }
	  
	  /* increment gamma spectra counter */
	  numGSpec++;
	}
    }

  if (numGSpec == 0)
    {
      delete[] numDisc;
      delete[] nIntReg;
      delete[] nPnts;
      delete[] discGammaE;
      delete[] discGammaI;
      delete[] intRegB;
      delete[] intRegT;
      delete[] contX;
      delete[] contY;
    }


  return numGSpec;
}  