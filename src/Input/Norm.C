/* $Id: Norm.C,v 1.2 1999-08-24 22:06:22 wilson Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Input: functions directly related to input of data 
 * xCheck: functions directly related to cross-checking the input
 *         against itself for consistency and completeness
 * Preproc: functions directly related to preprocessing of input
 *          prior to solution 
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 */

#include "Norm.h"

/****************************
 *********** Input **********
 ***************************/

/****** get a list of interval Norms ********/
/* called by Input::read(...) */
void Norm::getNorms(istream& input)
{
  char token[64];
  Norm *ptr = this;
  
  verbose(2,"Reading normalization of intervals.");
  
  /* read list of interval definitions until keyword "end" */
  clearComment(input);
  input >> token;
  while (strcmp(token,"end"))
    {
      ptr->next = new Norm(atof(token));
      memCheck(next,"Norm::getNorms(...): next");
      ptr = ptr->next;
      
      verbose(3,"Added normalization %g.",atof(token));
      
      clearComment(input);
      input >> token;
    }
  
}





