/* $Id: LibIdx.C,v 1.4 1999-08-24 22:06:18 wilson Exp $ */
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

ALARALib::LibIdx::LibIdx(int& nPar, int& nGroups, FILE*& binLib, int &libType)
{
  nParents = 0;
  nGroups = 0;
  kza = NULL;
  offset = NULL;

  if (binLib != NULL)
    {
      int parNum;
      int junkInt, nRxns, rxnNum;
      long junkLong, idxOffset;
      char buffer[64];
      
      /* make sure we are at the top of the library */
      fseek(binLib,0L,SEEK_SET);

      /* find index */
      fread(&idxOffset,SLONG,1,binLib);
      fseek(binLib,idxOffset,SEEK_SET);
      
      debug(0,"Skipped to index offset: %d",idxOffset);
      /* read number of parents and number of groups */
      fread(&libType,SINT,1,binLib);
      debug(0,"Library type: %c",libType);
      fread(&nParents,SINT,1,binLib);
      debug(0,"Number of parents: %d",nParents);
      fread(&nGroups,SINT,1,binLib);
      
      /* copy number of parents to passed reference */
      nPar = nParents;
      
      fread(&junkInt,SINT,1,binLib);
      fread(&junkLong,SLONG,1,binLib);
      fread(&junkInt,SINT,1,binLib);
      fread(&junkLong,SLONG,1,binLib);
      
      if (nParents>0)
	{
	  kza = new int[nParents];
	  memCheck(kza,"ALARALib::LibIdx::LibIdx(...) constructor: kza");

	  offset = new long[nParents];
	  memCheck(offset,"ALARALib::LibIdx::LibIdx(...) constructor: offset");
	  
	  for (parNum=0;parNum<nParents;parNum++)
	    {
	      fread(kza+parNum,SINT,1,binLib);
	      fread(&nRxns,SINT,1,binLib);
	      fread(offset+parNum,SLONG,1,binLib);
	      for (rxnNum=0;rxnNum<nRxns;rxnNum++)
		{
		  fread(&junkInt,SINT,1,binLib);
		  fread(&junkInt,SINT,1,binLib);
		  fread(buffer,1,junkInt,binLib);
		  fread(&junkLong,SLONG,1,binLib);
		}
	    }
	}
    }
}

ALARALib::LibIdx::LibIdx(const LibIdx& l)
{
  int parNum;
  
  kza = NULL;
  offset = NULL;

  nParents = l.nParents;
  if (nParents>0)
    {
      kza = new int[nParents];
      memCheck(kza,"ALARALib::LibIdx::LibIdx(...) copy constructor: kza");

      offset = new long[nParents];
      memCheck(offset,"ALARALib::LibIdx::LibIdx(...) copy constructor: offset");

      for (parNum=0;parNum<nParents;parNum++)
	{
	  kza[parNum] = l.kza[parNum];
	  offset[parNum] = l.offset[parNum];
	}
    }
}

ALARALib::LibIdx& ALARALib::LibIdx::operator=(const LibIdx& l)
{
  if (this == &l)
    return *this;

  int parNum;
  
  delete kza;
  delete offset;

  kza = NULL;
  offset = NULL;

  nParents = l.nParents;
  if (nParents>0)
    {
      kza = new int[nParents];
      memCheck(kza,"ALARALib::LibIdx::operator=(...): kza");

      offset = new long[nParents];
      memCheck(offset,"ALARALib::LibIdx::operator=(...): offset");

      for (parNum=0;parNum<nParents;parNum++)
	{
	  kza[parNum] = l.kza[parNum];
	  offset[parNum] = l.offset[parNum];
	}
    }

  return *this;

}

/****************************
 ********* Utility **********
 ***************************/
	  
/* routine to search the index and retrieve the offset for the binary
 * data library */
long ALARALib::LibIdx::search(int findKza, int min, int max)
{

  debug(3,"Search for %d between %d and %d",findKza,min,max);
  /* if max is -1, this is a new search */
  if (max==-1) 
    {

      debug(4,"Initiating search for %d in list of %d isotopes.",
	    findKza,nParents);
      /* set the maximum to the number of parents in the index */
      max=nParents-1;

      /* check to see if it is the last item or first item 
       * because the they might never be checked
       * with recursive integer averaging */
      /* this is only necessary at start of search:
       * all subsequent max or min will be tested before they are set */

      /* if matches the max */
      if (findKza == kza[max])
	return offset[max];
      /* if matches the min */
      if (findKza == kza[min])
	return offset[min];
    }

  /* when converged! 
   * a) max-min can never equal 0 with integer averaging
   * b) if max-min > 1, not converged
   * c) if max-min == 1, no match in set
   *      because we have already checked both the absolute extrema
   *      and each extreme as we recursed */
  if ((max-min)==1)
      /* return failed search flag = -1 */
      return -1;
  /* no convergence : recurse binary search */
  else
    {
      /* find midpoint */
      int mid = (max+min)/2;

      /* test for match at midpoint */
      if (findKza == kza[mid])
	return offset[mid];
      else if (findKza > kza[mid])
	return search(findKza,mid,max);
      else
	return search(findKza,min,mid);
    }
  
}    
  



