/* $Id: OutputFormat.h,v 1.11 2000-01-17 18:45:21 wilson Exp $ */
#include "alara.h"
#include <set>

/* ******* Class Description ************

This class is invoked as a linked list and is used to store the
definitions of the desired output resolutions, types and formatting.
The first element of each list has type OUTRES_HEAD (defined through
the resolution member), and contains no problem data.

 *** STL Typedef Definitions ***

 filenameList : set<char*,compare>
    This STL set is used to store a unique list of filenames.

 *** Static Class Members ***

 resolution : int
    This indicates the geometrical resolution of this output
    definition, i.e. whether it should print the interval, zone or
    mixture results.

 outTypes : int
    This is used as a bit-field by combining a number of predefined
    (Output_def.h) bits which indicate particular kinds of output
    responses or other information.

 normType : int
    This is used to store which kind of normalization (mass vs volume,
    including metric prefix) is being used for this output block.

 wdrFilenames : filenameList
    This list of filenames indicates how many WDR calculations are
    needed and which files to use for the WDR thresholds.

 actUnits : char*
    This string is used in output to display which activity units the
    results are being shown in (Ci vs. Bq).

 normUnits : char*
    This string is used in output to display which normalization units
    the results are being shown in (cm3 v. m3 v. g v. kg).

 actMult : double
    This stores the multiplication factor for the activity units.  If
    necessary (based on actUnits), it will store the conversion
    between Ci and Bq.

 next: OutputFormat*
    The next object in the linked-list.

 *** Member Functions ***

 * - Constructors and Destructors - *

 OutputFormat(int type=OUTRES_HEAD) 
    When called without arguments, the default constructor creates a
    blank list head with no problem data.  Otherwise, it sets the
    'resolution' to the first argument and initializes the 'next'
    pointer to NULL.

 OutputFormat(const OutputFormat&)
    This copy constructor copies 'resolution' and 'outTypes' but sets
    'next' to NULL.

 ~OutputFormat()
    This destructor destroys the whole list be deleting 'next'.

 Output& operator=(const OutputFormat&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the object will continue to be part of the
    same list unless explicitly changed.

 * - Input - *

 OutputFormat* getOutFmts(istream&)
    This function reads the next output description from the input
    file attached to the stream reference of the argument.

 * - Postproc - *

 void write(Volume*,Mixture*,Loading*,CoolingTime*,int)
    This function steps through the linked list of output descriptions
    and writes each one in sequence by calling the write() function on
    the list of intervals, zones or mixtures, as determined by the
    'resolution' member. The last argument is the kza number for the
    target isotope for which the current invocation is being called.

 */

#ifndef _OUTPUTFORMAT_H
#define _OUTPUTFORMAT_H

#define OUTRES_HEAD 0
#define OUTRES_INT  1
#define OUTRES_ZONE 2
#define OUTRES_MIX  3

#include "Output_def.h"

extern const char *OUTPUT_RES;
extern const char *OUTPUT_TYPES;

class compare {
public:
  bool operator()(const char *s,const char *t)const
    { return strcmp(s,t) < 0;
    }
};

typedef set<char*,compare> filenameList;

class OutputFormat
{
protected:

  int resolution, outTypes, normType;
  filenameList wdrFilenames;
  char *actUnits, *normUnits;
  double actMult;

  OutputFormat *next;

public:
  /* Service */
  OutputFormat(int type=OUTRES_HEAD);
  OutputFormat(const OutputFormat&);
  ~OutputFormat();

  OutputFormat& operator=(const OutputFormat&);

  /* Input */
  OutputFormat* getOutFmts(istream&);

  /* Postproc */
  void write(Volume*,Mixture*,Loading*,CoolingTime*,int targetKza=0);

};

#endif
