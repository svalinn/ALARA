#ifndef __EXXSECSIZE_H__
#define __EXXSECSIZE_H__

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
  ExXsecSize(const std::string& loc, int xsSize, int otherSize);

};

#endif
