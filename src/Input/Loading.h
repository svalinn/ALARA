/* $Id: Loading.h,v 1.11 2002-08-05 20:23:16 fateneja Exp $ */
#include "alara.h"

#ifndef _LOADING_H
#define _LOADING_H

#include "Input_def.h"

/** \brief This class is invoked as a linked list and cross-references 
 *         the zones with a particular material loading. 
 *
 *  The single object of class Input has a list of Loadings.  The first element
 *  in each list is named IN_HEAD (defined in Input.h), and contains no problem 
 *  data.
 */

class Loading
{
protected:
  char 
    /// The descriptive name of this zone, used in other input to
    /// cross-reference this zone with intervals, for example.
    *zoneName,
    
    /// The name of the mixture to be used in this zone.
    /** This should match (it will be checked) the descriptive name of one of 
        the objects in the Mixture list. */
    *mixName;

  Mixture 
    /// A pointer to the mixture in this zone.
    *mixPtr;

  double
    /// The volume of the zone, for averaging the results.
    volume;

  int 
    /// The number of components in this zone.
    nComps;

  Result
    /// A linked list of final number densities for each component and for
    /// the total.
    *outputList;

  double 
    /// The response totalled over the whole zone is stored in an array to
    /// enable the printing of a table of totals.
    *total;

  Loading 
    /// The next Loading object in this list.
    *next;

public:
  ///  Default constructor 
  /**  When called with no arguments, it creates an blank list head with 
       no problem data.  Otherwise, it creates and fills the storage for
       'zoneName' and 'mixName' and initializes next to NULL. */
  Loading(char* name=IN_HEAD, char *mxName=NULL);
  
  /// Copy constructor 
  /** This constructor is identical to default constructor.  Therefore,
      'zoneName' and 'mixName' are copied, but the 'outputList' and
      'total' are initialized to NULL successive list item 'next' is
      not. */
  Loading(const Loading&);
  
  /// Inline destructor deletes the storage for the names and destroys
  /// the whole list by deleting 'next'.
  ~Loading();
  
  /// Overloaded assignment operator
  /** This assignment operator behaves similarly to the copy
      constructor.  The correct implementation of this operator must
      ensure that previously allocated space is returned to the free
      store before allocating new space into which to copy the
      object. Note that 'next' is NOT copied, the left hand side object
      will continue to be part of the same list unless explicitly
      changed. */
  Loading& operator=(const Loading&);

  /// This function reads the list of material loadings from the input 
  /// file attached to the passed input stream.  
  /** The material loadings are expected to appear as a single group 
      in the correct order.  This function reads from the first loading 
      until keyword 'end'. */
  void getMatLoading(istream&);
  
  /// Read the explicit list of zones which will be solved (or skipped)
  /// for this problem.  
  /** These lists are cross-referenced with the  master list in 
      xCheck(). */
  void getSolveList(istream&);

  /// This function accounts accounts for the list of explicitly solved
  /// and skipped zones, and does a simple cross check to ensure that all
  /// referenced mixtures exist.
  /** If a mixture does not exist, it will generate an error and the
      program will halt.  The argument is a pointer to a Mixture object,
      and should be the head of the mixture list. */
  void xCheck(Mixture*,Loading*,Loading*);

  /// This function tallies the result list pointed to by the first
  /// argument into this object's result list.
  /** The tallying is weighted by the second argument.  This is used to
      tally the results of each interval in to the total zone results, 
      weighted by the interval volume. */
  void tally(Result* , double);

  /// This function is responsible for writing the results to standard
  /// output.
  /** The first argument indicates which kind of response is being 
      written and the second indicates whether a mixture component
      breakdown was requested.  The third argument points to the list of
      after-shutdown cooling times.  The fourth argument indicates the
      kza of the target isotope for a reverse calculation and is simply
      passed on the the Result::write().  The final argument indicates
      what type of normalization is being used, so that the correct
      output information can be given. */
  void write(int,int,CoolingTime*,int,int);

  /// Inline access to the 'next' pointer.  
  Loading* advance() { return (this!= NULL)?next:(Loading*)NULL; };

  /// Inline access to the descriptive name.
  /** Note that this is not entirely robust as it simply returns the char*
      pointer, the contents of which might be (but shouldn't be) 
      subsequently changed. */
  char* getName() { return zoneName;};

  /// Inline access to the mixture name.  
  /** Note that this is not entirely robust as it simply returns the char*
      pointer, the contents of which might be (but shouldn't be) 
      subsequently changed. */
  char* getMix() { return mixName;};

  // NEED COMMENT
  double getVol() { return volume;};

  /// Inline function to determine if this is the head of the list.
  /** Returns boolean comparison of 'zoneName' and IN_HEAD. */
  int head() { return (!strcmp(zoneName,IN_HEAD));};

  // NEED COMMENT
  int nonEmpty() { return (next != NULL || !head()); };

  /// Search function to find the material loading of a given zone,
  /// specified by the argument which is compared to 'zoneName'.  
  /** If found, returns a pointer to that zone loading description,
      otherwise, NULL. */
  Loading* findZone(char *);

  /// Search function to find the first occurence of a material loading
  /// using a given mixture, specified by the argument which is
  /// compared to 'mixName'.  
  /** If found, returns a pointer to that zone loading description, 
      otherwise, NULL.  Note: this returns the first occurence after a the
      object through which it is called - if called through the list head,
      this is the absolute occurence. By successive calls through the object
      returned by the previous call, this will find all the occurences. */
  Loading* findMix(char *);

  /// Function to count the number of elements in this list, i.e. the
  /// number of zones.
  int numZones();

  /// This function is used for reverse calculations to clear the values
  /// of the outputList, since the results are not cummulative across
  /// subsequent targets.
  void resetOutList();
};

#endif
