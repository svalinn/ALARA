/* $Id: Input.h,v 1.7 2003-01-13 04:34:57 fateneja Exp $ */
#include "alara.h"

#ifndef INPUT_H
#define INPUT_H

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
      /// This is the input stream pointer for this element of the stack.
      istream *strm;

      /// This points to the next element in the stack.
      istreamStack *last;

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
      istreamStack& operator<<(istream*);

      /// This implements the pop operation.
      istreamStack& operator>>( istream*& instream);

    } 
  
  /// This is the stack of input stream pointers which allows for the
  /// nested including of input file.
  streamStack;

  /// This is the current input stream pointer.
  istream *input;

  /// A pointer to the problem's geometry object.
  Geometry *inGeom;

  /// The head of the list of mixture definitions for this problem.
  Mixture *mixListHead;
    
  /// The mixture defintion most recently added to the list.
  Mixture *mixList;

  /// The head of the linked list of flux definitions for this problem.
  Flux *fluxListHead;
    
  /// The flux definition most recently added to the list.
  Flux *fluxList;
  
  /// The head of the linked list of pulsing history definitions for
  /// this problem.
  History *historyListHead;
    
  /// The pulsing history definitions most recently added to the list.
  History *historyList;

  /// The head of the linked list of schedule defintions for this problem.
  Schedule *schedListHead;
    
  /// The schedule definition most recently added to the list.
  Schedule *schedList;
  
  /// The head of the linked list of dimensional definitions for this problem.
  Dimension *dimListHead;
      
  /// The dimensional definition most recently added to the list.
  Dimension *dimList;
  
  /// The linked list of cooling time definitions for this problem.
  CoolingTime *coolList;
  
  /// The linked list of interval definitions for this problem.
  Volume *volList;
  
  /// The linked list of interval normalizations for this problem.
  Norm *normList;
  
  /// The linked list of material laoding definitions for this problem.
  Loading *loadList;
    
  /// A linked list explicitly defining which zones should be sovled.
  Loading *solveList;
    
  /// A linked list defining which zones should be skipped.
  Loading *skipList;
  
  /// The head of the linked list of output defintions.
  OutputFormat *outListHead;
  
public:
  /// Default constructor
  Input(char* inputFname = NULL);

  /// Default destructor
  ~Input();
  
  /// The main outside function to parse an input file.
  void read();
  
  /// This function performs some simple cross-checking of the input for
  /// self-consistency and completeness.
  void xCheck();
  
   /// This function performs some pre-processing of the data, converting
   /// and copying information from one type of object to another.
  void preProc(Root*&, topSchedule*&);

  /// This function performs the post-processing of the results.
  void postProc(Root*);

   /// This function is similar to the globally defined
   /// clearComment(...).
  void clearIncludeComment();
};

#endif
