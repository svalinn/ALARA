/* $Id: Input.h,v 1.5 2000-01-18 02:37:35 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is the container for the entire problem input.  All the
various input lists are members of this class and are only accessible
through this class.

 *** Locally Defined Classes ***

 istreamStack

    This simple FILO stack type data structure allows infinte nesting
    of included input files.  When required, a new input stream
    pointer is pushed onto the stack.  When that stream is finished,
    it is popped off the stack and the next input stream is attached
    to the current input stream.  The current input stream should NOT
    be on the stack.

    *** Class Members ***

    strm : istream
       This is the input stream pointer for this element of the stack.

    last : istreamStack*
       This points to the next element in the stack.

    *** Member Functions ***

    * - Constructors & Destructors - *
    
    istreamStack(istream*, istreamStack*)
       This default constructor sets 'input' to the first argument and
       'last' to the second.  The second or both arguments can be
       missing, in which case both pointers are set to NULL.

    ~istreamStack()
       Inline destructor deletes the istream item.

    * - Stack Operations - *

    * - Push - *
    istreamStack& operator<<(const istream*)
       This implements the push operation.  The argument is pushed
       onto the stack and the pointer to the new head of the stack is
       returned.

    * - Pop - *
    istreamStack& operator>>(istream*&)
       This implements the pop operation.  The top of the stack is
       popped and the reference argument is set to the 'input' member
       of the next stack element.  A pointer to the new head of the
       stack is returned.  If the stack is empty, everything is set to
       NULL.

 * END istreamStack DESCRIPTION *

 *** Class Members ***

 streamStack : istreamStack
    This is the stack of input stream pointers which allows for the
    nested including of input file.

 input : istream*
    This is the current input stream pointer.

 inGeom : Geometry*
    A pointer to the problem's geometry object.

 mixListHead : Mixture*
    The head of the list of mixture definitions for this problem.

 mixList : Mixture*
    The mixture defintion most recently added to the list.

 fluxListHead : Flux*
    The head of the linked list of flux definitions for this problem.

 fluxList : Flux*
    The flux definition most recently added to the list.

 historyListHead : History*
    The head of the linked list of pulsing history definitions for
    this problem.

 historyList : History*
    The pulsing history definitions most recently added to the list.

 schedListHead : Schedule*
    The head of the linked list of schedule defintions for this problem.
 
 schedList : Schedule*
    The schedule definition most recently added to the list.

 dimListHead : Dimension*
    The head of the linked list of dimensional definitions for this problem.

 dimList : Dimension*
    The dimensional definition most recently added to the list.

 coolList : CoolingTime*
    The linked list of cooling time definitions for this problem.

 volList : Volume*
    The linked list of interval definitions for this problem.

 normList : Norm*
    The linked list of interval normalizations for this problem.

 loadList : Loading*
    The linked list of material laoding definitions for this problem.

 solveList : Loading*
    A linked list explicitly defining which zones should be sovled.

 skipList : Loading*
    A linked list defining which zones should be skipped.

 outListHead : OutputFormat*
    The head of the linked list of output defintions.

 *** Member Functions ***

 * - Constructors & Destructors - *

 Input(char*)
    This default constructor creates a new Input object to contain the
    problem input and connects it to an input stream.  If called with
    no arguments, the input is read from stdin, otherwise it is read
    from the filename specified in the argument.  The constructor
    initializes all the different members, with the list tails
    pointing to the list heads.

 ~Input()
    The default destructor deletes all the members, assuming that all
    the lists will be destroyed by their own destructors.

 * - Input - *

 void read()
    The main outside function to parse an input file.  This function
    reads a token, calls tokenType(...) to parse it to an integer, and
    then acts appropriately, calling the individual class methods to
    read a certain object.
 
 * - xCheck - *

 void xCheck()
    This function performs some simple cross-checking of the input for
    self-consistency and completeness.  Error and warning messages
    will notify the user of problems.  Some eraly preprocessing is
    also done here.
 
 * - Preproc - *

 void preProc(Root*&, topSchedule*&)
    This function performs some pre-processing of the data, converting
    and copying information from one type of object to another.  In
    particular, dimensional definitions are converted to interval
    lists, Mixture definitions are converted to lists of root isotopes
    and the schedule and pulsing history information is converted to
    the calculation phase objects used to represent these elements of
    the problem.  The two pointers passed by reference are set to
    point to the root isotope list and the schedule hierarchy,
    respectively.

 * - Postproc - *

 void postProc()
    This function performs the post-processing of the results, first
    tallying all the results across the intervals, zones and mixtures,
    through the list of mixtures.  It then calls on the output format
    objects to create the appropriate output.

 * - Utility - *

 void clearIncludeComment()
    This function is similar to the globally defined
    clearComment(...), but this instance allows for files to be
    included with a C preprocessor type directive: #include
    "included.file.name".
 

 */

#ifndef _INPUT_H
#define _INPUT_H

class Input
{
protected:
  
  class istreamStack
    {
    protected:
      istream *strm;
      istreamStack *last;

    public:
      istreamStack(istream *instream=NULL, istreamStack *stackItem=NULL)
	{ strm=instream;  last = stackItem; };

      /* Push */
      istreamStack& operator<<(istream*);

      /* Pop */
      istreamStack& operator>>( istream*& instream);

    } streamStack;

  istream *input;

  Geometry *inGeom;
  Mixture *mixListHead, *mixList;
  Flux *fluxListHead, *fluxList;
  History *historyListHead, *historyList;
  Schedule *schedListHead, *schedList;
  Dimension *dimListHead, *dimList;
  CoolingTime *coolList;
  Volume *volList;
  Norm *normList;
  Loading *loadList, *solveList, *skipList;
  OutputFormat *outListHead;
  
public:
  /* Service */
  Input(char* inputFname = NULL);
  ~Input();
  
  /* Input */
  void read();
  
  /* xCheck */
  void xCheck();
  
  /* Preproc */
  void preProc(Root*&, topSchedule*&);

  /* Postproc */
  void postProc(Root*);

  /* Utility */
  void clearIncludeComment();
};

#endif
