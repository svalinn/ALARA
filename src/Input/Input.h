/* $Id: Input.h,v 1.6 2002-08-05 20:23:16 fateneja Exp $ */
#include "alara.h"

#ifndef _INPUT_H
#define _INPUT_H

/** \brief This class is the container for the entire problem input.  
 *
 *  All the various input lists are members of this class and are only
 *  accessible through this class. 
 */

class Input
{
protected:
  
  /** \brief This simple FILO stack type data structure allows infinte 
   *         nesting of included input files.
   *
   *  When required, a new input stream pointer is pushed onto the stack.
   *  When that stream is finished, it is popped off the stack and the 
   *  next input stream is attached to the current input stream.  The 
   *  current input stream should NOT be on the stack.
   */

  class istreamStack
    {
    protected:
      istream 
        /// This is the input stream pointer for this element of the stack.
        *strm;

      istreamStack 
        /// This points to the next element in the stack.
        *last;

    public:
      /// Default constructor
      /** This default constructor sets 'input' to the first argument and
          'last' to the second.  The second or both arguments can be
          missing, in which case both pointers are set to NULL. */
      istreamStack(istream *instream=NULL, istreamStack *stackItem=NULL)
	{ strm=instream;  last = stackItem; };

      // NEED COMMENT There is a comment for an inline destructor,
      //              but I counldn't find one for class istreamStack

      /// This implements the push operation.
      /** The argument is pushed onto the stack and the pointer to the new
          head of the stack is returned. */
      istreamStack& operator<<(istream*);

      /// This implements the pop operation.
      /** The top of the stack is popped and the reference argument is set
          to the 'input' member of the next stack element.  A pointer to the
          new head of the stack is returned.  If the stack is empty, 
          everything is set to NULL. */
      istreamStack& operator>>( istream*& instream);

    } 
  
    /// This is the stack of input stream pointers which allows for the
    /// nested including of input file.
    streamStack;

  istream 
    /// This is the current input stream pointer.
    *input;

  Geometry 
    /// A pointer to the problem's geometry object.
    *inGeom;

  Mixture 
    /// The head of the list of mixture definitions for this problem.
    *mixListHead, 
    
    /// The mixture defintion most recently added to the list.
    *mixList;

  Flux 
    /// The head of the linked list of flux definitions for this problem.
    *fluxListHead, 
    
    /// The flux definition most recently added to the list.
    *fluxList;
  
  History 
    /// The head of the linked list of pulsing history definitions for
    /// this problem.
    *historyListHead, 
    
    /// The pulsing history definitions most recently added to the list.
    *historyList;

  Schedule 
    /// The head of the linked list of schedule defintions for this problem.
    *schedListHead, 
    
    /// The schedule definition most recently added to the list.
    *schedList;
  
  Dimension 
    /// The head of the linked list of dimensional definitions for this problem.
    *dimListHead, 
    
    /// The dimensional definition most recently added to the list.
    *dimList;
  
  CoolingTime 
    /// The linked list of cooling time definitions for this problem.
    *coolList;
  
  Volume 
    /// The linked list of interval definitions for this problem.
    *volList;
  
  Norm 
    /// The linked list of interval normalizations for this problem.
    *normList;
  
  Loading 
    /// The linked list of material laoding definitions for this problem.
    *loadList, 
    
    /// A linked list explicitly defining which zones should be sovled.
    *solveList, 
    
    /// A linked list defining which zones should be skipped.
    *skipList;
  
  OutputFormat 
    /// The head of the linked list of output defintions.
    *outListHead;
  
public:
  /// Default constructor
  /** This constructor creates a new Input object to contain the
      problem input and connects it to an input stream.  If called with
      no arguments, the input is read from stdin, otherwise it is read
      from the filename specified in the argument.  The constructor
      initializes all the different members, with the list tails
      pointing to the list heads. */
  Input(char* inputFname = NULL);

  /// Default destructor
  /** Deletes all the members, assuming that all the lists will be destroyed
      by their own destructors. */
  ~Input();
  
  /// The main outside function to parse an input file.
  /** This function reads a token, calls tokenType(...) to parse it to an 
      integer, and then acts appropriately, calling the individual class 
      methods to read a certain object. */
  void read();
  
  /// This function performs some simple cross-checking of the input for
  /// self-consistency and completeness.
  /** Error and warning messages will notify the user of problems.  Some
      early preprocessing is also done here. */
  void xCheck();
  
   /// This function performs some pre-processing of the data, converting
   /// and copying information from one type of object to another.
   /** In particular, dimensional definitions are converted to interval
       lists, Mixture definitions are converted to lists of root isotopes
       and the schedule and pulsing history information is converted to
       the calculation phase objects used to represent these elements of
       the problem.  The two pointers passed by reference are set to
       point to the root isotope list and the schedule hierarchy,
       respectively. */
  void preProc(Root*&, topSchedule*&);

  /// This function performs the post-processing of the results.
  /** It first tallies all the results across the intervals, zones and 
      mixtures, through the list of mixtures.  It then calls on the output 
      format objects to create the appropriate output. */
  void postProc(Root*);

   /// This function is similar to the globally defined
   /// clearComment(...).
   /** However, this instance allows for files to be
       included with a C preprocessor type directive: #include
       "included.file.name". */
  void clearIncludeComment();
};

#endif
