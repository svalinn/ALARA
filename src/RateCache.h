/* $Id: RateCache.h,v 1.5 2003-01-13 04:34:28 fateneja Exp $ */
#include "alara.h"

#ifndef RATECACHE_H
#define RATECACHE_H

#define CACHE_SIZE 64
#define BLANK_KZA 9999999

/** \brief This class stores implements a fixed size cache for scalar
 *         reaction rates.  (One of these objects is associated with 
 *         each flux in the problem.)
 *  
 *  The fixed size cache (see CACHE_SIZE below) keeps track of all the
 *  reactions for CACHE_SIZE isotopes.  An array of CacheListPtrs is
 *  sorted by the kza number of the base isotope.  Each CacheListPtr 
 *  has a pointer to a CacheData object which contains an array of 
 *  rates, one for each reaction that this base isotope sees.
 *
 *  The CacheData objects are also stored as a doubly-linked list, 
 *  sorted by which object was used most recently.  When the cache is 
 *  full, the least recently used isotope is deleted from the cache in 
 *  order to add the new object.  The CacheData objects store the 
 *  index of the CacheListPtr which points to it, in order to replace 
 *  the value of the CacheListPtr with those the new isotope.  At each 
 *  addition, the array of CacheListPtrs is sorted using a simple bubble 
 *  sort [O(N)].
 *
 *  Some simple analysis has shown that CACHE_SIZE=64 will only have a
 *  cache miss about 1% of the time for a typical problem.  Cache misses
 *  are most likely when changing root isotope, and especially when the
 *  new root isotope has a Z very different from that of the old root
 *  isotope.
 */

class RateCache
{
 protected:

  /** \brief These objects contain the actual data for the cache.
   */
  class CacheData 
    {
    public:
      /// This is an index into the array RateCache::kzaList,
      /// indicating which CacheListPtr object points to this 
      /// CacheData object.
      int kzaIdx;

      /// This dynamically allocated array of doubles has scalar nuclear
      /// reaction rates for each reaction for this base isotope
      /// (including the total destruction rate).
      double *cache;

      /// Pointer to implement the doubly linked list
      CacheData *next;
        
      /// Pointer to implement the doubly linked list
      CacheData *prev;
      
      /// Default constructor
      /** This constructor sets kzaIdx to -1, and all the pointers to 
          NULL. */
      CacheData() : kzaIdx(-1), cache(NULL), next(NULL), prev(NULL) {};

      /// This, if the single argument is greater than 0, allocates
      /// that many doubles and points 'cache' at this new array.
      CacheData(int);

      /// Default destructor deletes the whole linked list by deleting
      /// 'next'.
      ~CacheData() { delete next; delete[] cache;};

    }
  /// This points to the least recently used data.  
  /** It therefore serves as a pointer to the whole list from one 
      side. */
  *oldestData, 
    
  /// This points to the most recently used data.
  /** It also serves as a pointer to the whole list form the other 
      side. */
  *lastUsedData;
  
  class CacheListPtr 
    {
    public:
      /// Hash integer defining the isotope.  This is the search/sort 
      /// key for the array of these objects.
      int kza;

      /// This is a pointer to the CacheData object which corresponds to
      /// this isotope.
      CacheData *dataPtr;
      
      /// Default Constructor
      /** This constructor uses an initialization list sets kza
          to the value of the single argument (with BASE_KZA being the
          default) and sets dataPtr to NULL. */
      CacheListPtr(int baseKza=BLANK_KZA) : kza(baseKza), dataPtr(NULL) {};   
    } 
  /// This static array is a sorted list of CacheListPtrs, initially
  /// filled with kza=BLANK_KZA in all locations.
  kzaList[CACHE_SIZE];
  
  /// This function performs a simple binary search for the single
  /// argument on the kzaList array.
  RateCache::CacheData* search(int);

  /// This function adds a new CacheData object, preforms all the
  /// management of the linked list related to this, inserts the kza
  /// reference into kzaList, and performs a bubble sort on kzaList.
  RateCache::CacheData* add(int,int);

 public:
  /// Default constructor 
  /** This constructor relies on default constructors for all of
      its members execpt oldestData and lastUsedData, which are set
      to NULL. */
  RateCache() : oldestData(NULL), lastUsedData(NULL) {};

  /// This default destructor relies on default destructors for all
  /// members, but deletes the linked list by deleting oldestData.
  ~RateCache();

  /// This function searches for the isotope indicated by the first
  /// argument and the reaction indexed by the second argument.
  double read(int,int);

  /// This function sets the entry in the cache for the isotope indcated
  /// by the first argument and the reaction indexed by the second
  /// argument.
  void set(int,int,int,double);
};
#endif

