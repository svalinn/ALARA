#ifndef EXFILEOPEN_H
#define EXFILEOPEN_H

#include "../FeindNs.h"
#include "Exception.h"

#include <string>

/// Exception class for errors involving files that FEIND is unable to open
/** This exception is usually thrown by the parsers, when users give names of
 *  files which do not exist.
 */
class FEIND::ExFileOpen : public Exception
{
 public:

  /// Main constructor
  /** \param[in] loc
   *  The location where the error occurred.
   *
   *  \param[in] fn
   *  The name of the file that could not be opened.
   */
  ExFileOpen(const std::string& loc, const std::string& fn);

};

#endif

