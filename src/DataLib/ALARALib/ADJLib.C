/* $Id: ADJLib.C,v 1.6 1999-08-24 22:06:18 wilson Exp $ */
#include "ADJLib.h"

/* open an existing library */
ADJLib::ADJLib(char* fname)
  : ALARALib(fname,DATALIB_ADJOINT)
{ 
  totalXSection = NULL;
  E[0] = 0;
  E[1] = 0;
  E[2] = 0;
  thalf = 0;

}
  
/* create a new library */
ADJLib::ADJLib(char* fname, char* adjLibName) :
  ALARALib(fname,DATALIB_ALARA)
{
  normBinLib = binLib;

  char adjointLibName[256], adjointIdxName[256];
  strcpy(adjointLibName,adjLibName);
  strcat(adjointLibName,".lib");
  strcpy(adjointIdxName,adjLibName);
  strcat(adjointIdxName,".idx");

  binLib = fopen(adjointLibName,"wb");
  tmpIdx.open(adjointIdxName, ios::out);

  offset = 0;
  daugList = NULL;

  totalXSection = new float[nGroups+1];
  E[0] = 0;
  E[1] = 0;
  E[2] = 0;
  thalf = 0;
  xSection = new float[nGroups+1];

  if (normBinLib != NULL)
    build();

  close(nParents,DATALIB_ADJOINT,adjointIdxName);
  fclose(normBinLib);

}

void ADJLib::copyHead()
{
  float *grpBnds, *grpWeights;

  /* initialize these values for new library */
  offset = 0;
  nParents = daugList->count();

  /* save place for offset of index */
  offset += fwrite(&offset,SLONG,1,binLib)*SLONG;

  /* save place for number of parents */
  offset += fwrite(&nParents,SLONG,1,binLib)*SLONG;

  /* write number of neutron groups */
  offset += fwrite(&nGroups,SINT,1,binLib)*SINT;

  /* read grpBound info */
  /* make sure we are at the correct place in the normal library */
  fseek(normBinLib,offset,SEEK_SET);

  /* read and write grpBnds flag */
  fread(&grpBnds,SINT,1,normBinLib);
  tmpIdx << -1 << "\t" << offset << endl;
  offset += fwrite(&grpBnds,SINT,1,binLib)*SINT;
  /* read and write grpBnds data */
  if (grpBnds != NULL)
    {
      grpBnds = new float[nGroups+1];
      fread(grpBnds,SFLOAT,nGroups+1,normBinLib);
      offset += fwrite(grpBnds,SFLOAT,nGroups+1,binLib)*SFLOAT;
      delete grpBnds;
      grpBnds = NULL;
    }

  /* read and write grpWeights flag */
  fread(&grpWeights,SINT,1,normBinLib);
  tmpIdx << 0 << "\t" << offset << endl;
  offset += fwrite(&grpWeights,SINT,1,binLib)*SINT;
  /* read and write grpWeights data */
  if (grpWeights != NULL)
    {
      grpWeights = new float[nGroups];
      fread(grpWeights,SFLOAT,nGroups,normBinLib);
      offset += fwrite(grpWeights,SFLOAT,nGroups,binLib)*SFLOAT;
      delete grpWeights;
      grpWeights = NULL;
    }

  debug(4,"Copied header info.");
}

void ADJLib::getForwardData(int kza)
{
  int nRxns, checkKza, rxnNum, gNum, emittedLen;
  long normOffset = idx->search(kza);
  
  /* initialize data */
  for (gNum=0;gNum<=nGroups;gNum++)
    totalXSection[gNum] = 0;
  E[0] = 0;
  E[1] = 0;
  E[2] = 0;
  thalf = 0;

  if (normOffset > 0)
    {
      fseek(normBinLib,normOffset,SEEK_SET);
      verbose(5,"Found data for %d at offset %d",kza,normOffset);

      /* get isotope info */
      fread(&checkKza,SINT,1,normBinLib);
      fread(&nRxns,SINT,1,normBinLib);
      fread(&thalf,SFLOAT,1,normBinLib);
      fread(E,SFLOAT,3,normBinLib);

      if (thalf>0)
	totalXSection[nGroups] = log(2)/thalf;

      /* Read info for each daughter */
      for (rxnNum=0;rxnNum<nRxns;rxnNum++)
	{
	  fread(&checkKza,SINT,1,normBinLib);
	  fread(&emittedLen,SINT,1,normBinLib);
	  fread(emitted,1,emittedLen,normBinLib);
	  emitted[emittedLen] = '\0';
	  fread(xSection,SFLOAT,nGroups+1,normBinLib);
	  if (strcmp(emitted,"x"))
	    for (gNum=0;gNum<nGroups;gNum++)
	      totalXSection[gNum] += xSection[gNum];
	}
    }

}      


void ADJLib::writeData(DaugItem *daug)
{
  int kza, parKza, nRxns;
  long normOffset;

  int checkKza,emittedLen;

  kza = daug->getKza();
  nRxns = daug->countRxns();

  verbose(2,"Writing entry for %d (%d)",kza,offset);

  /* member offset is used in ALARALib::readData(...) */
  getForwardData(kza);

  /* write parent isotope info */
  tmpIdx << kza << "\t" << nRxns << "\t" << offset << endl;
  offset += fwrite(&kza,SINT,1,binLib)*SINT;
  offset += fwrite(&nRxns,SINT,1,binLib)*SINT;
  offset += fwrite(&thalf,SFLOAT,1,binLib)*SFLOAT;
  offset += fwrite(E,SFLOAT,3,binLib)*SFLOAT;
  offset += fwrite(totalXSection,SFLOAT,nGroups+1,binLib)*SFLOAT;

  while ( (normOffset = daug->getNextReaction(parKza)) )
    {
      fseek(normBinLib,normOffset,SEEK_SET);
      
      fread(&checkKza,SINT,1,normBinLib);
      fread(&emittedLen,SINT,1,normBinLib);
      fread(emitted,1,emittedLen,normBinLib);
      emitted[emittedLen] = '\0';
      fread(xSection,SFLOAT,nGroups+1,normBinLib);

      tmpIdx << "\t" << parKza << "\t" << emitted 
	     << "\t" << offset << endl;
      
      offset+=fwrite(&parKza,SINT,1,binLib)*SINT;
      offset+=fwrite(&emittedLen,SINT,1,binLib)*SINT;
      offset+=fwrite(emitted,1,emittedLen,binLib);
      offset+=fwrite(xSection,SFLOAT,nGroups+1,binLib)*SFLOAT;

    }
}
  

void ADJLib::build()
{
  int libType;
  int parNum, parKza, daugKza,emittedLen;
  int junkInt, nRxns, rxnNum;
  long junkLong, idxOffset, rxnOffset;
  
  /* make sure we are at the top of the library */
  fseek(normBinLib,0L,SEEK_SET);
  
  /* find index */
  fread(&idxOffset,SLONG,1,normBinLib);
  fseek(normBinLib,idxOffset,SEEK_SET);
  
  debug(0,"Skipped to index offset: %d",idxOffset);
  /* read number of parents and number of groups */
  fread(&libType,SINT,1,normBinLib);
  debug(0,"Library type: %c",libType);
  fread(&nParents,SINT,1,normBinLib);
  debug(0,"Number of parents: %d",nParents);
  fread(&nGroups,SINT,1,normBinLib);
  
  fread(&junkInt,SINT,1,normBinLib);
  fread(&junkLong,SLONG,1,normBinLib);
  fread(&junkInt,SINT,1,normBinLib);
  fread(&junkLong,SLONG,1,normBinLib);
  
  if (nParents>0)
    {
      /* parse entire index */
      for (parNum=0;parNum<nParents;parNum++)
	{
	  /* get required parent info */
	  fread(&parKza,SINT,1,normBinLib);
	  fread(&nRxns,SINT,1,normBinLib);
	  fread(&junkLong,SLONG,1,normBinLib);
	  for (rxnNum=0;rxnNum<nRxns;rxnNum++)
	    {
	      /* get daughter info for each reaction */
	      fread(&daugKza,SINT,1,normBinLib);
	      fread(&emittedLen,SINT,1,normBinLib);
	      fread(emitted,1,emittedLen,normBinLib);
	      fread(&rxnOffset,SLONG,1,normBinLib);

	      /* save parent/daughter info */
	      if (daugList == NULL)
		daugList = new DaugItem(daugKza,parKza,rxnOffset);
	      else
		daugList->add(daugKza,parKza,rxnOffset);
	    }
	}
    }
  
  /* copy binary file header: group boundaries and weights */
  copyHead();
  
  /* scroll through list of possible daughters */
  DaugItem *ptr = daugList;
  while (ptr != NULL)
    {
      /* write the reaction list for each daughter */
      writeData(ptr);
      /* get next daughter */
      ptr = ptr->advance();
    }
  
}

    
