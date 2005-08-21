#ifndef __LIBDEFINE_H__
#define __LIBDEFINE_H__

#include <string>
#include <vector>

#include "FeindNs.h"

class FEIND::LibDefine
{
 public:
  std::vector< std::string > Args;
  FEINDFormat Format;
};

#endif
