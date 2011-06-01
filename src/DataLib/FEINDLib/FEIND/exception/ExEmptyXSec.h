#ifndef __EXEMPTYXSEC_H__
#define __EXEMPTYXSEC_H__

#include "../FeindNs.h"
#include "Exception.h"

#include <string>

/// Exception object for operations performed on empty cross-sections
/** This exception class is thrown any time a XSec object is not initialized,
 *  and an attempt is made to perform some operation on it.
 */
class FEIND::ExEmptyXSec : public Exception
{
public:
  /// Main constructor
  /** \param[in] loc
   *  The location where the error occurred.
   */
  ExEmptyXSec(const std::string& loc);
};

#endif
