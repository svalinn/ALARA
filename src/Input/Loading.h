/* $Id: Loading.h,v 1.6 1999-08-24 22:06:21 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list and cross-references the zones
with a particular material loading.  The single object of class Input
has a list of Loadings.  The first element in each list is named
IN_HEAD (defined in Input.h), and contains no problem data.


 *** Class Members ***

 zoneName : char*
    The descriptive name of this zone, used in other input to
    cross-reference this zone with intervals, for example.

 mixName : char*
    The name of the mixture to be used in this zone.  This should
    match (it will be checked) the descriptive name of one of the
    objects in the Mixture list.

 mixPtr : Mixture*
    A pointer to the mixture in this zone.

 volume : double
    The volume of the zone, for averaging the results.

 nComps : int
    The number of components in this zone.

 outputList : Result*
    A linked list of final number densities for each component and for
    the total.

 total : double*
    The response totalled over the whole zone is stored in an array to
    enable the printing of a table of totals.

 next : Loading*
    The next Loading object in this list.

 *** Member Functions ***

 Loading(char*, char*)
    Default constructor: when called with no arguments, it creates an
    blank list head with no problem data.  Otherwise, it creates and
    fills the storage for 'zoneName' and 'mixName' and initializes
    next to NULL.

 Loading(const Loading&)
    Copy constructor is identical to default constructor.  Therefore,
    'zoneName' and 'mixName' are copied, but the successive list item
    'next' is not.

 ~Loading()
    Inline destructor deletes the storage for the names and destroys
    the whole list by deleting 'next'.
 
 Loading& operator=(const Loading&);
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the object will continue to be part of the
    same list unless explicitly changed.

 * - Input - *

 void getMatLoading(istream&)
    Read the list of material loadings from the input file attached
    to the passed input stream.  The material loadings are expected
    to appear as a single group in the correct order.  This function
    reads from the first loading until keyword 'end'.

 * - xCheck - *

 void xCheck(Mixture*)
    Simple cross-check to ensure that all referenced mixtures exist.
    If a mixture does not exist, it will generate an error and the
    program will halt.  The argument is a pointer to a Mixture
    object, and should be the head of the mixture list.

 * - Postproc - *

 void tally(Result*,double)
    This function tallies the result list pointed to by the first
    argument into this object's result list.  The tallying is weighted
    by the second argument.  This is used to tally the results of each
    interval in to the total zone results, weighted by the interval
    volume.

 void write(int,int,CoolingTime*)
    This function is responsible for writing the results to standard
    output.  The first argument indicates which kind of response is
    being written, the second indicates whether a mixture component
    breakdown was requested, and the last points to the list of
    after-shutdown cooling times.

 * - Utility - *

 Loading* advance()
    Inline access to the 'next' pointer.

 char* getName()
    Inline access to the descriptive name.  Note that this is not
    entirely robust as it simply returns the char* pointer, the
    contents of which might be (but shouldn't be) subsequently
    changed.

 char* getMix()
    Inline access to the mixture name.  Note that this is not
    entirely robust as it simply returns the char* pointer, the
    contents of which might be (but shouldn't be) subsequently
    changed.

 int head()
    Inline function to determine if this is the head of the list.
    Returns boolean comparison of 'zoneName' and IN_HEAD.

 Loading* findZone(char *) 
   Search function to find the material loading of a given zone,
   specified by the argument which is compared to 'zoneName'.  If
   found, returns a pointer to that zone loading description,
   otherwise, NULL.

 Loading* findMix(char *)
    Search function to find the first occurence of a material loading
    using a given mixture, specified by the argument which is
    compared to 'mixName'.  If found, returns a pointer to that zone
    loading description, otherwise, NULL.  Note: this returns the
    first occurence after a the object through which it is called -
    if called through the list head, this is the absolute occurence.
    By successive calls through the object returned by the previous
    call, this will find all the occurences.

 int numZones()
    Function to count the number of elements in this list, i.e. the
    number of zones.

 */

#ifndef _LOADING_H
#define _LOADING_H

#include "Input_def.h"

class Loading
{
protected:
  char *zoneName,*mixName;
  Mixture *mixPtr;
  double volume;
  int nComps;
  Result *outputList;
  double *total;

  Loading *next;

public:
  /* Service */
  Loading(char* name=IN_HEAD, char *mxName=NULL);
  Loading(const Loading&);
  ~Loading();
  
  Loading& operator=(const Loading&);

  /* Input */
  void getMatLoading(istream&);

  /* xCheck */
  void xCheck(Mixture*);

  /* Postproc */
  void tally(Result* , double);
  void write(int,int,CoolingTime*,int);

  /* Utility */
  Loading* advance() { return (this!= NULL)?next:(Loading*)NULL; };
  char* getName() { return zoneName;};
  char* getMix() { return mixName;};
  int head() { return (!strcmp(zoneName,IN_HEAD));};
  Loading* findZone(char *);
  Loading* findMix(char *);
  int numZones();
  void resetOutList();
};



#endif
