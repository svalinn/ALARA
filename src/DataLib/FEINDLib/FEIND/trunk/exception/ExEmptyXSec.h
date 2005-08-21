#ifndef __EXEMPTYXSEC_H__
#define __EXEMPTYXSEC_H__

#include "../FeindNs.h"
#include "Exception.h"

#include <string>

class FEIND::ExEmptyXSec : public Exception
{
public:
  ExEmptyXSec(const std::string& loc);
};

#endif
