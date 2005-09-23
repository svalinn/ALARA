#ifndef __LIBDEFINE_H__
#define __LIBDEFINE_H__

#include <string>
#include <vector>

#include "FeindNs.h"

/// A LibDefine object is used to load libraries into the RamLib
/** The LibDefine class simply stores information about what format a nuclear
 *  data library is in, and what arguments need to be passed to that library's
 *  parser.
 */
class FEIND::LibDefine
{
 public:
  /// A list of arguments that are sent to a libraries parser.
  std::vector< std::string > Args;

  /// The format of the data library.
  FEINDFormat Format;
};

#endif
