/* $Id: OutputFormat.h,v 1.16 2002-08-05 20:23:19 fateneja Exp $ */
#include "alara.h"
#include <set>

#ifndef _OUTPUTFORMAT_H
#define _OUTPUTFORMAT_H

#define OUTRES_HEAD 0
#define OUTRES_INT  1
#define OUTRES_ZONE 2
#define OUTRES_MIX  3

#include "Output_def.h"

extern const char *OUTPUT_RES;
extern const char *OUTPUT_TYPES;

typedef std::set<char*,compare> filenameList;

/** \brief This class is invoked as a linked list and is used to store 
 *         the definitions of the desired output resolutions, types and 
 *         formatting.
 *
 *  The first element of each list has type OUTRES_HEAD (defined through
 *  the resolution member), and contains no problem data.
 */
class OutputFormat
{
protected:

  int 
    /// This indicates the geometrical resolution of this output
    /// definition.
    /** I.e. whether it should print the interval, zone or
        mixture results. */
    resolution, 
    
    /// This is used as a bit-field
    /** It combins a number of predefined (Output_def.h) bits which 
        indicate particular kinds of output responses or other 
        information. */
    outTypes, 
    
    /// This is used to store which kind of normalization (mass vs volume,
    /// including metric prefix) is being used for this output block.
    normType;

  filenameList 
    /// This STL set is used to store a unique list of filenames.    
    wdrFilenames;

  char 
    /// This string is used in output to display which activity units the
    /// results are being shown in (Ci vs. Bq).
    *actUnits, 
    
    /// This string is used in output to display which normalization units
    /// the results are being shown in (cm3 v. m3 v. g v. kg).
    *normUnits;

  double 
    /// This stores the multiplication factor for the activity units.  
    /** If necessary (based on actUnits), it will store the conversion
        between Ci and Bq. */
    actMult;

  GammaSrc 
    /// NEED COMMENT
    *gammaSrc, 
    
    // NEED COMMENT
    *contactDose;

  OutputFormat 
    /// The next object in the linked-list.
    *next;

public:
  /// Default constructor
  /** When called without arguments, the default constructor creates a
      blank list head with no problem data.  Otherwise, it sets the
      'resolution' to the first argument and initializes the 'next'
      pointer to NULL. */
  OutputFormat(int type=OUTRES_HEAD);
  
  /// This copy constructor copies 'resolution' and 'outTypes' but sets
  /// 'next' to NULL.
  OutputFormat(const OutputFormat&);
  
  /// This destructor destroys the whole list be deleting 'next'.
  ~OutputFormat();

  /// Overloaded assignment operator
  /** The correct implementation of this operator must ensure that
      previously allocated space is returned to the free store before
      allocating new space into which to copy the object. Note that
      'next' is NOT copied, the object will continue to be part of the
      same list unless explicitly changed. */
  OutputFormat& operator=(const OutputFormat&);

  /// This function reads the next output description from the input
  /// file attached to the stream reference of the argument.
  OutputFormat* getOutFmts(istream&);

  /// This function steps through the linked list of output descriptions
  /// and writes each one in sequence 
  /** It does this by calling the write() function on the list of intervals,
      zones or mixtures, as determined by the 'resolution' member. The last
      argument is the kza number for the target isotope for which the 
      current invocation is being called. */
  void write(Volume*,Mixture*,Loading*,CoolingTime*,int targetKza=0);
};

#endif
