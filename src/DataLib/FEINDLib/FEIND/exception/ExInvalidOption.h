#ifndef __EXINVALIDOPTION_H__
#define __EXINVALIDOPTION_H__

#include "../FeindNs.h"
#include "Exception.h"

#include <string>

/// Exception class for invalid option errors
/** This exception should be thrown indicate that a parser has been given an 
 *  invalid option and can not continue.
 */
class FEIND::ExInvalidOption : public Exception
{
public:

  /// Main constructor takes a location identifier, and the invalid option
  ExInvalidOption(const std::string& loc, const std::string& option);
};

#endif
