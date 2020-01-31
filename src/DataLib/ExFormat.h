#ifndef EXFORMAT_H
#define EXFORMAT_H

#include "FeindNs.h"
#include "Exception.h"

#include <string>

/// Exception class for unknown formats
/** This exception is thrown when FEIND is asked to load a library in an
 *  unknown format
 */
class FEIND::ExFormat : public FEIND::Exception
{
 public:
  
  /// Main constructor
  /** \param[in] loc
   *  The location where the error occurred.
   *
   *  \param[in] format
   *  The invalid format that was given
   */
  ExFormat(const std::string& loc, FEINDFormat format);
};

#endif
