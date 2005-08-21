#ifndef __EXFILEOPEN_H__
#define __EXFILEOPEN_H__

#include "../FeindNs.h"
#include "Exception.h"

#include <string>

class FEIND::ExFileOpen : public Exception
{
 public:
  ExFileOpen(const std::string& loc, const std::string& fn);

};

#endif

