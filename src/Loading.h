/* $Id: Loading.h,v 1.14 2003-01-14 05:01:18 wilsonp Exp $ */
#include "alara.h"

#ifndef LOADING_H
#define LOADING_H

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
  /// The descriptive name of this zone, used in other input to
  /// cross-reference this zone with intervals, for example.
  char *zoneName;
    
    /// The name of the mixture to be used in this zone.
    /** This should match (it will be checked) the descriptive name of one of 
        the objects in the Mixture list. */
  char *mixName;

  /// A pointer to the mixture in this zone.
  Mixture *mixPtr;

  /// The volume of the zone, for averaging the results.
  double volume;

  /// User-defined (actual) volume of zone for dose calculation.
  double userVol;

  /// Flag indicating whether user defines uservol.
  bool userVolFlag;
 
  /// The number of components in this zone.
  int nComps;

  /// A linked list of final number densities for each component and for
  /// the total.
  Result *outputList;

  /// The response totalled over the whole zone is stored in an array to
  /// enable the printing of a table of totals.
  double *total;
 
  /// The next Loading object in this list.
  Loading *next;

public:
  ///  Default constructor 
  Loading(const char* name=IN_HEAD, const char *mxName=NULL, bool inUserVolFlag=FALSE, double inUserVol=0.0);
  
  /// Copy constructor 
  Loading(const Loading&);
  
  /// Default destructor deletes the storage for the names and destroys
  /// the whole list by deleting 'next'.
  ~Loading();
  
  /// Overloaded assignment operator
  Loading& operator=(const Loading&);

  /// This function reads the list of material loadings from the input 
  /// file attached to the passed input stream.  
  void getMatLoading(istream&);
  
  /// Read the explicit list of zones which will be solved (or skipped)
  /// for this problem.  
  void getSolveList(istream&);

  /// This function accounts accounts for the list of explicitly solved
  /// and skipped zones, and does a simple cross check to ensure that all
  /// referenced mixtures exist.
  void xCheck(Mixture*,Loading*,Loading*);

  /// This function tallies the result list pointed to by the first
  /// argument into this object's result list.
  void tally(Result* , double);

  /// This function is responsible for writing the results to standard
  /// output.
  void write(int,int,CoolingTime*,int,int);

  /// Inline access to the 'next' pointer.  
  Loading* advance() { return next; };

  /// Inline access to the descriptive name.
  char* getName() { return zoneName;};

  /// Inline access to the mixture name.  
  char* getMix() { return mixName;};

  // NEED COMMENT
  double getVol() { return volume;};

  /// Inline function to determine if this is the head of the list.
  int head() { return (!strcmp(zoneName,IN_HEAD));};

  // NEED COMMENT
  int nonEmpty() { return (next != NULL || !head()); };

  /// Search function to find the material loading of a given zone,
  /// specified by the argument which is compared to 'zoneName'.  
  Loading* findZone(char *);

  /// Search function to find the first occurence of a material loading
  /// using a given mixture, specified by the argument which is
  /// compared to 'mixName'.  
  Loading* findMix(char *);

  /// Function to count the number of elements in this list, i.e. the
  /// number of zones.
  int numZones();

  /// This function is used for reverse calculations to clear the values
  /// of the outputList, since the results are not cummulative across
  /// subsequent targets.
  void resetOutList();

  /// Access function for userVol. Returns userVol if userVolFlag is set
  /// else returns zero.
  double getUserVol() {return userVolFlag?userVol:0.0;};
};

#endif
