#include "Input.h"

/* Push */
Input::istreamStack& Input::istreamStack::operator<<(istream* instream)
{ 
  last = new istreamStack(*this); 
  strm = instream; 

  return *this;
}

/* Pop */
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
