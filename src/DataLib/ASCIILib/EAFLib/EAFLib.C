#include "EAFLib.h"

EAFLib::EAFLib(char* transFname, char* decayFname) : ASCIILib(DATALIB_EAF)
{
  if (transFname != NULL && decayFname != NULL)
    {
      inTrans.open(transFname, ios::in);
      inDecay.open(decayFname, ios::in);

      makeBinLib();
    }

}

EAFLib::~EAFLib()
{

  /* delete arrays here that were dimensioned with
   * EAF relevant constants */
  int rxnNum;

  for (rxnNum=0;rxnNum<MAXEAFRXNS;rxnNum++)
    {
      delete xSection[rxnNum];
      delete emitted[rxnNum];
    }
  delete xSection;
  delete emitted;
  delete transKza;

  delete decayKza;
  delete bRatio;

  inTrans.close();
  inDecay.close();
}

void extract(char* input, float* value)
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

int delKza(float rxnType)
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
  char *gasList = "tdpah";
  char *gasEmitted = "x    ";
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
  
  debug(5,"Found %d reaction paths for %d.",nTRxns,zak);
  
  return zak;
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
  int pFlag, aFlag, decyNum;
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
  
  /* read line */
  /* betaA --> gammaE --> alphaE */
  inDecay.getline(buffer,MAXLINELENGTH);
  /* extract data */
  extract(buffer,E);
  extract(buffer+22,E+1);
  extract(buffer+44,E+2);
  
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
  
  /* skip spectra information */
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
