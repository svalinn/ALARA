/* $Id: Result.h,v 1.20 2008-08-06 17:38:10 phruksar Exp $ */
#include "alara.h"

#ifndef RESULT_H
#define RESULT_H

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
  /// This indicates how many double precision results are stored for
  /// each output isotope.
  /** This is equal to the number of cooling times plus one (for the
      shutdown result). */
  static int nResults;

  /// This standard C file descriptor is used for a binary dump of the
  /// results during the course of the calculation.
  /** It is written to after the solution of each root (target) 
      isotope, and then read, one root isotope at a time, during 
      post-processing. */
  static FILE* binDump;

  /// This value (-1) is written to the dump file following each root
  /// isotope's record
  /** It is used as a delimiter when reading the file during 
      post-processing. */
  static const int delimiter;

  /// This value is set by OutputFormat::actMult and holds the activity
  /// unit multiplier appropriate for the activity units of this output
  /// block.
  static double actMult;
  
  /// This value is a multiplier for the results to account for varying
  /// metric prefixes in the normalization of the results.
  static double metricMult;

  // NEED COMMENT
  static GammaSrc *gammaSrc;

  /// String to print as reminder of current output type
  static char* outReminderStr;

  /// This indicates the kza number of the output isotope which this
  /// data is for.
  int kza;

  static int cooltime_units;

  /// This is an array of results, one for each cooling time and at
  /// shutdown.
  double *N;

  /// This is the result of the next isotope in this list of results.
  Result* next;
  
public:
  /// Inline function initializes the number of results to be stored in
  /// each Result object.
  static void setNResults(int numRes)
    { nResults = numRes; };

  // NEED COMMENTS
  static void setGammaSrc(GammaSrc *setGammaSrc)
    { gammaSrc = setGammaSrc; };

  /// Default constructor
  Result(int setKza = 0, Result* nxtPtr=NULL);  
  
  /// Copy constructor
  Result(const Result&);
  
  // NEED COMMENT
  Result(int,float*);
  
  /// The inline destructor deletes the storage for 'N' and then
  /// destroys the whole result list by deleting 'next'.
  ~Result()
    { delete[] N; delete next; next=NULL;};

  /// Overloaded assignment operator
  Result& operator=(const Result&);

  /// This function scans the linked list for the KZA value given in 
  /// the argument and returns a pointer to the object with that KZA.
  Result* find(int);
  
  /// Inline function deletes a list of results.
  void clear()
    { delete next; next = NULL; };

  /// This function adds the results passed in the argument on an
  /// element-by-element basis, to the 'N' array contained here.  
  void tally(double*,double scale=1.0);
  
  /// This function parses a whole chain and adds the solution from
  /// the appropriate nodes to the current solution vector.  
  void tallySoln(Chain*,topScheduleT*);

  /// This function adds the results in 'this' list to the
  /// list passed in the first argument
  void postProcTarget(Result*, Mixture*);
  
  /// This function adds the results in 'this' list to the list passed 
  /// in the first argument.
  void postProcList(Result*, Mixture*, int);
  
  /// This function steps through the list from which it was called,
  /// searching for a matching entry for the list passed in the first
  /// argument.
  void postProc(Result&, double density=1.0);
  
  /// This does all the work of writing out a table of results, stepping
  /// through the list of results.  
  void write(int, int, Mixture*, CoolingTime*, double*&, double volume_mass=1,Volume* volPtr=NULL);
  
  /// This function is used to set actMult from the first argument and
  /// metricMult by interpretation of the second argument.
  static void setNorm(double,int,int);

  int getCooltimeMode() const;

  /// This function is used to set outReminderStr so that each table
  /// makes it clear what is being written.  Corresponding function to query
  /// the string.
  static void setReminderStr(char*);
  static char* getReminderStr(){ return outReminderStr;};

  /// This function opens and initializes the binary dump file used
  /// throughout the solution and postprocessing.
  static void initBinDump(const char*);
  
  /// This function dumps the value of nResults to the binary dump file.
  static void dumpHeader();
  
  /// This function checks for the existence of a binary dump file.
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

