/* $Id: istreamStack.C,v 1.3 2003-01-13 04:35:00 fateneja Exp $ */
#include "Input.h"

/** The argument is pushed onto the stack and the pointer to the new
    head of the stack is returned. */
 Input::istreamStack& Input::istreamStack::operator<<(istream* instream)
{ 
  last = new istreamStack(*this); 
  strm = instream; 

  return *this;
}


/** The top of the stack is popped and the reference argument is set
    to the 'input' member of the next stack element.  A pointer to the 
    new head of the stack is returned.  If the stack is empty, everything
    is set to NULL. */
Input::istreamStack& Input::istreamStack::operator>>( istream*& instream)
{
  instream = strm;
  
  if (last != NULL)
    {
      istreamStack* tmp = last;
      strm = tmp->strm;
      last = tmp->last;
      delete tmp;
    }
  else
    strm = NULL;

  return *this;
}
