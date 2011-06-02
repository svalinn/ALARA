#ifndef PARSER_H
#define PARSER_H

#include "FeindNs.h"
#include "LibDefine.h"

/// A pure abstract class base class for all Parsers.
/** So far the interface represented by this class simply contains a single
 *  function to load the contents of a library into memory.
 */
class FEIND::Parser
{
public:

  /// Abstract function to load the contents of a library into the RamLib.
  virtual void LoadLibrary() = 0;

};

#endif
