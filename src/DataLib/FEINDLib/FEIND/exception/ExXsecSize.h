#ifndef EXXSECSIZE_H
#define EXXSECSIZE_H

#include "../FeindNs.h"
#include "Exception.h"

#include <string>

/// Exception class to handler errors with operations on xsections.
/** Specifically, this class should be used when some operation is performed
 *  on a XSec object which involves vectors of differing sizes. For example,
 *  adding to XSec objects together requires that they both have the same
 *  group structure. This exception object is used to report these errors.
 */
class FEIND::ExXsecSize : public Exception
{
public:
  
  /// Main Constructor
  /** \param[in] loc
   *  The location where the error occurred.
   *
   *  \param[in] xsSize
   *  The size of the XSection object
   *
   *  \param[in] otherSize
   *  The size of the other obect (XSec or vector)
   */
  ExXsecSize(const std::string& loc, int xsSize, int otherSize);

};

#endif
