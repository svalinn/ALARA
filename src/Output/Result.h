/* $Id: Result.h,v 1.14 2002-08-05 20:23:19 fateneja Exp $ */
#include "alara.h"

#ifndef _RESULT_H
#define _RESULT_H

/* conversion from eV to Joules */
#define EV2J 1.6e-19

/** \brief This class is used to store the results of each computation.
 *       
 *         A linked list of Result objects will make up two (2) of the 
 *         dimensions for each of the 4-D (interval * input isotope * 
 *         output isotope * cooling time) result arrays.  The head of 
 *         each list is an object of class ResultList, derived from 
 *         this class and indicating the input isotope corresponding to
 *         this list.  Each interval will have a linked list of 
 *         ResultList objects, one for each of the input isotopes which 
 *         exist in that interval.
 */

class Result
{
protected:
  static int 
    /// This indicates how many double precision results are stored for
    /// each output isotope.
    /** This is equal to the number of cooling times plus one (for the
        shutdown result). */
    nResults;

  static FILE*
    /// This standard C file descriptor is used for a binary dump of the
    /// results during the course of the calculation.
    /** It is written to after the solution of each root (target) 
        isotope, and then read, one root isotope at a time, during 
        post-processing. */
    binDump;

  static const int 
    /// This value (-1) is written to the dump file following each root
    /// isotope's record
    /** It is used as a delimiter when reading the file during 
        post-processing. */
    delimiter;

  static double 
    /// This value is set by OutputFormat::actMult and holds the activity
    /// unit multiplier appropriate for the activity units of this output
    /// block.
    actMult;

  static double 
    /// This value is a multiplier for the results to account for varying
    /// metric prefixes in the normalization of the results.
    metricMult;

  static GammaSrc 
    // NEED COMMENT
    *gammaSrc;

  int 
    /// This indicates the kza number of the output isotope which this
    /// data is for.
    kza;

  double 
    /// This is an array of results, one for each cooling time and at
    /// shutdown.
    *N;

  Result* 
    /// This is the result of the next isotope in this list of results.
    next;
  
public:
  /// Inline function initializes the number of results to be stored in
  /// each Result object.
  static void setNResults(int numRes)
    { nResults = numRes; };

  // NEED COMMENTS
  static void setGammaSrc(GammaSrc *setGammaSrc)
    { gammaSrc = setGammaSrc; };

  /// Default constructor
  /** When called with no arguments, the default constructor sets 'kza'
      and 'next' to 0 and NULL, respectively.  Otherwise, they are set,
      respectively, by the arguments. */
  Result(int setKza = 0, Result* nxtPtr=NULL);
  
  
  /// Copy constructor
  /** The copy constructor copies 'kza', makes an element-by-element
      copy of 'N' and sets next to NULL. */
  Result(const Result&);
  
  // NEED COMMENT
  Result(int,float*);
  
  /// The inline destructor deletes the storage for 'N' and then
  /// destroys the whole result list by deleting 'next'.
  ~Result()
    { delete next; delete N; };

  /// Overloaded assignment operator
  /** The correct implementation of this operator must ensure that
      previously allocated space is returned to the free store before
      allocating new space into which to copy the object. Note that
      'next' is NOT copied, the object will continue to be part of the
      same list unless explicitly changed. */
  Result& operator=(const Result&);

  /// This function scans the linked list for the KZA value given in 
  /// the argument and returns a pointer to the object with that KZA.
  /** If none is found, a new one is created in the right list location,
      and this pointer is returned. */
  Result* find(int);
  
  /// Inline function deletes a list of results.
  void clear()
    { delete next; next = NULL; };

  /// This function adds the results passed in the argument on an
  /// element-by-element basis, to the 'N' array contained here.  
  /** Since Result objects are initialized with N=[0], and 'find(...)' 
      creates the new object, this function only needs to do the 
      summation.  The second argument defaults to 1, and is the weight
      used to tally this particular result i.e. a density or a volume.
      */
  void tally(double*,double scale=1.0);
  
  /// This function parses a whole chain and adds the solution from
  /// the appropriate nodes to the current solution vector.  
  /** The 'appropriate nodes' are determined by polling some 
      parameters of the chain.  This is used during the solution phase 
      of ALARA. */
  void tallySoln(Chain*,topScheduleT*);

  /// This function adds the results in 'this' list to the
  /// list passed in the first argument
  /** This function is used during postprocessing to form the final 
      solutions of reverse calculations, but only if the kza refered to
      in the current item of 'this' list is contained in the list of
      root isotopes in the mixture referred to in the second argument.
      */
  void postProcTarget(Result*, Mixture*);
  
  /// This function adds the results in 'this' list to the list passed 
  /// in the first argument.
  /** This function is used during postprocessing to form the final 
      solutions of forward calculations, , but only if `the kza referred
      to in the third argument is contained in the list of root isotopes
      in the mixture referred to in the second argument. */
  void postProcList(Result*, Mixture*, int);
  
  /// This function steps through the list from which it was called,
  /// searching for a matching entry for the list passed in the first
  /// argument.
  /** It then tallies the results to that object with a weighting
      defined by the second argument, which defaults to 1. */
  void postProc(Result&, double density=1.0);
  
  /// This does all the work of writing out a table of results, stepping
  /// through the list of results.  
  /** Based on the first argument, it queries the data library for the
      scalar multiplier of this response.  It normalizes this multiplier
      by the last argument (e.g. the volume of a zone) and then prints
      out the formatted output for each isotope and each cooling time, 
      pointed to by the second argument.  It simultaneously sets the 
      total for this point at each cooling time, at the pointer passed by
      reference in the third argument. */
  void write(int, int, Mixture*,CoolingTime*, double*&, double volume_mass=1);
  
  /// This function is used to set actMult from the first argument and
  /// metricMult by interpretation of the second argument.
  static void setNorm(double,int);

  /// This function opens and initializes the binary dump file used
  /// throughout the solution and postprocessing.
  static void initBinDump(char*);
  
  /// This function dumps the value of nResults to the binary dump file.
  static void dumpHeader();
  
  /// This function checks for the existence of a binary dump file.
  /** If not found, opens one with the default name 'alara.dmp'. */

  static void xCheck();

  /// This function flushes the buffer and resets the file pointer to the
  /// beginning of the file, to be read in the post-processing step.
  static void resetBinDump();
  
  // NEED COMMENT
  static void closeBinDump() { fclose(binDump); };
  
  /// This function writes the results stored in 'this' entire list to the
  /// binary dump file.
  void writeDump();
  
  
  /// This function reads the results from the binary dump file into a
  /// new list of results.
  void readDump();

};
  
#endif

