/* $Id: RateCache.C,v 1.3 2003-01-13 04:34:28 fateneja Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "RateCache.h"

/****************************
 ********* Service **********
 ***************************/

RateCache::CacheData::CacheData(int size)
  : kzaIdx(-1), cache(NULL), next(NULL), prev(NULL)
{ 
  if (size>0) 
    cache = new double[size]; 
  for (int ix=0;ix<size;cache[ix++]=-1);
}

RateCache::~RateCache()
{
  delete oldestData;
}

/** It returns a pointer to the CacheData object which correpsonds to
    this isotope.  If the isotope is not found, it returns NULL. It
    is a protected function since it should only be called from
    RateCache::read and RateCache::set. */
RateCache::CacheData* RateCache::search(int baseKza)
{
  int min=0,max=CACHE_SIZE-1,mid;

  if (baseKza == kzaList[max].kza)
    return kzaList[max].dataPtr;

  if (baseKza == kzaList[min].kza)
    return kzaList[min].dataPtr;

  if (baseKza < kzaList[max].kza && baseKza > kzaList[min].kza)
    while (min < max-1)
      {
	mid = (min + max)/2;
	if (baseKza == kzaList[mid].kza)
	  return kzaList[mid].dataPtr;
	else 
	  if (baseKza > kzaList[mid].kza)
	    min = mid;
	  else
	    max = mid;
      }
  
  return NULL;
}

/** If this isotope does not currently exist in the cache, a value
    of -1 is returned.  Otherwise, the scalar reaction rate
    corresponding to this reaction of this isotope is returned. */
double RateCache::read(int baseKza, int pathNum)
{
  double rate=-1;
  CacheData *dataPtr = search(baseKza);

  if (dataPtr)
    {
      /* if not already at the end of the list */
      if (lastUsedData != dataPtr)
	{
	  if (oldestData == dataPtr)
	    /* delete this from the beginning or... */
	    oldestData = dataPtr->next;
	  else
	    /* ... from middle of the list */
	    dataPtr->prev->next = dataPtr->next;
	  
	  dataPtr->next->prev = dataPtr->prev;

	  /* put this at the end of the list */
	  lastUsedData->next = dataPtr;
	  dataPtr->prev = lastUsedData;

	  /* point the last used data pointer at the right place */
	  lastUsedData = dataPtr;
	  lastUsedData->next = NULL;
	}

      rate = lastUsedData->cache[pathNum];
    }
  
  return rate;
  
}

/** The value of the scalar reaction rate is given in the
    fourth argument.  If there is currently no cache entry for this
    isotope, the third argument is used to set the size of the new
    CacheData object which is created for this isotope. */
void RateCache::set(int baseKza, int nRates, 
		    int pathNum, double rate)
{
  CacheData *dataPtr = search(baseKza);

  if (dataPtr == NULL)
    dataPtr = add(baseKza,nRates);

  dataPtr->cache[pathNum] = rate;

}

/** It is only called from RateCache::set when search returns a
    NULL. */
RateCache::CacheData* RateCache::add(int baseKza, int nRates)
{
  int idx = CACHE_SIZE;
  CacheData *tmpData;
  
  /* if the cache is empty */
  if (oldestData == NULL)
    {
      oldestData = new CacheData(nRates);
      lastUsedData = oldestData;
      idx=0;
    }
  else
    {
      /* search for first empty spot in cache (i.e. if not full) */
      while (idx > 0 && kzaList[idx-1].kza == BLANK_KZA) idx--;
      
      /* if the cache if full */
      if (idx == CACHE_SIZE)
	{
	  idx = oldestData->kzaIdx;
	  
	  /* remove oldest data from list */
	  tmpData = oldestData->next;
	  oldestData->next = NULL;
	  delete oldestData;
	  oldestData = tmpData;
	}
      
      /* add new data to list */
      tmpData = new CacheData(nRates);
      tmpData->prev = lastUsedData;
      lastUsedData->next = tmpData;
      lastUsedData = tmpData;
    }

  /* bubble search for correct location */
  while (idx < CACHE_SIZE-1 && baseKza > kzaList[idx+1].kza)
    {
      if (kzaList[idx+1].dataPtr != NULL)
	kzaList[idx+1].dataPtr->kzaIdx = idx;
      kzaList[idx] = kzaList[idx+1];
      idx++;
    }
  while (idx > 0 && baseKza < kzaList[idx-1].kza)
    {
      if (kzaList[idx-1].dataPtr != NULL)
	kzaList[idx-1].dataPtr->kzaIdx = idx;
      kzaList[idx] = kzaList[idx-1];
      idx--;
    }


  kzaList[idx].kza = baseKza;
  kzaList[idx].dataPtr = lastUsedData;
  lastUsedData->kzaIdx = idx;

  return lastUsedData;

}
