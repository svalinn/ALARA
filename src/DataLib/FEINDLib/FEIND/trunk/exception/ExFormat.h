#ifndef __EXFORMAT_H__
#define __EXFORMAT_H__

#include "../FeindNs.h"
#include "Exception.h"

#include <string>

class FEIND::ExFormat : public FEIND::Exception
{
 public:
  ExFormat(const std::string& loc, FEINDFormat format);
};

#endif
