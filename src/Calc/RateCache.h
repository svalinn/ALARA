/* $Id: RateCache.h,v 1.1 2000-01-30 06:38:41 wilson Exp $ */
#include "alara.h"

#ifndef _RATECACHE_H
#define _RATECACHE_H

#define CACHE_SIZE 64
#define BLANK_KZA 9999999

class RateCache
{
 protected:
  class CacheData 
    {

    protected:

    public:
      /* data members */
      int kzaIdx;
      double *cache;
      CacheData *next,*prev;
      
      /* member functions */
      CacheData() : kzaIdx(-1), cache(NULL), next(NULL), prev(NULL) {};
      CacheData(int);

      ~CacheData() { delete next; };

    } *oldestData, *lastUsedData;
  
  class CacheListPtr 
    {

    protected:

    public:
      /* data members */
      int kza;
      CacheData *dataPtr;
      
      /* member functions */
      CacheListPtr(int baseKza=BLANK_KZA) : kza(baseKza), dataPtr(NULL) {};
      
    } kzaList[CACHE_SIZE];
  
  RateCache::CacheData* search(int);
  RateCache::CacheData* add(int,int);

 public:
  RateCache() : oldestData(NULL), lastUsedData(NULL) {};
  ~RateCache();

  double read(int,int);
  void set(int,int,int,double);

  
};
#endif

