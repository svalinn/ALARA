/* $Id: RateCache.h,v 1.2 2000-02-11 20:55:19 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class stores implements a fixed size cache for scalar reaction
rates.  (One of these objects is associated with each flux in the
problem.)

The fixed size cache (see CACHE_SIZE below) keeps track of all the
reactions for CACHE_SIZE isotopes.  An array of CacheListPtrs is
sorted by the kza number of the base isotope.  Each CacheListPtr has a
pointer to a CacheData object which contains an array of rates, one
for each reaction that this base isotope sees.

The CacheData objects are also stored as a doubly-linked list, sorted
by which object was used most recently.  When the cache is full, the
least recently used isotope is deleted from the cache in order to add
the new object.  The CacheData objects store the index of the
CacheListPtr which points to it, in order to replace the value of the
CacheListPtr with those the new isotope.  At each addition, the array
of CacheListPtrs is sorted using a simple bubble sort [O(N)].

Some simple analysis has shown that CACHE_SIZE=64 will only have a
cache miss about 1% of the time for a typical problem.  Cache misses
are most likely when changing root isotope, and especially when the
new root isotope has a Z very different from that of the old root
isotope.

 *** Locally Defined Classes ***

 CacheData 

    These objects contain the actual data for the cache.

    *** Class Members ***

    kzaIdx : int
       This is an index into the array RateCache::kzaList, indicating
       which CacheListPtr object points to this CacheData object.

    cache : double*
       This dynamically allocated array of doubles has scalar nuclear
       reaction rates for each reaction for this base isotope
       (including the total destruction rate).

    next, prev : CacheData*
       Pointers to implement the doubly linked list.

    *** Member Functions ***

    * - Constructors & Destructors - *

    CacheData()
       This default constructor sets kzaIdx to -1, and all the
       pointers to NULL.

    CacheData(int)
       This if the single argument is greater than 0, it allocates
       that many doubles and points 'cache' at this new array.

    ~CacheData()
       Default destructor deletes the whole linked list by deleting
       'next'.

 * END CacheData DESCRIPTION *

 CacheListPtr 

    These objects are allocated as an array to implement an
    alternative sorting for the linked list.

    *** Class Members ***

    kza : int
       Hash integer defining the isotope.  This is the search/sort key
       for the array of these objects.

    dataPtr : CacheData*
       This is a pointer to the CacheData object which corresponds to
       this isotope.

    *** Member Functions ***

    * - Constructors & Destructors - *

    CacheListPtr(int=BASE_KZA)
       This default constructor uses an initialization list sets kza
       to the value of the single argument (with BASE_KZA being the
       default) and sets dataPtr to NULL.

 * END CacheListPtr DESCRIPTION *

 *** Class Members ***

 oldestData : CacheData*
    This points to the least recently used data.  It also therefore
    serves as a pointer to the whole list from one side.
    
 lastUsedData : CacheData*
    This points to the most recently used data.  It also therefore
    serves as a pointer to the whole list form the other side.

 kzaList[CACHE_SIZE] : CacheListPtr
    This static array is a sorted list of CacheListPtrs, initially
    filled with kza=BLANK_KZA in all locations.

 *** Protected Member Functions ***

 CacheData* search(int)
 
    This function performs a simple binary search for the single
    argument on the kzaList array.  It returns a pointer to the
    CacheData object which correpsonds to this isotope.  If the
    isotope is not found, it returns NULL.  It is a protected function
    since it should only be called from RateCache::read and
    RateCache::set.

 CacheData* add(int)
    This function adds a new CacheData object, preforms all the
    management of the linked list related to this, inserts the kza
    reference into kzaList, and performs a bubble sort on kzaList.  It
    is only called from RateCache::set when search returns a NULL.

 *** Public Member Functions ***

 * - Constructors & Destructors - *

 RateCache()
    This default constructor relies on default constructors for all of
    its members execpt oldestData and lastUsedData, which are set to
    NULL.

 ~RateCache()
    This default destructor relies on default destructors for all
    members, but deletes the linked list by deleting oldestData.

 * - Access - *

 double read(int,int)
    This function searches for the isotope indicated by the first
    argument and the reaction indexed by the second argument.  If this
    isotope does not currently exist in the cache, a value of -1 is
    returned.  Otherwise, the scalar reaction rate corresponding to
    this reaction of this isotope is returned.

 void set(int,int,int,double)
    This function sets the entry in the cache for the isotope indcated
    by the first argument and the reaction indexed by the second
    argument.  The value of the scalar reaction rate is given in the
    fourth argument.  If there is currently no cache entry for this
    isotope, the third argument is used to set the size of the new
    CacheData object which is created for this isotope.
 

*/

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

