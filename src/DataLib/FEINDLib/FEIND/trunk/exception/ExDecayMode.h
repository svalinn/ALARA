#ifndef __EXDECAYMODE_H__
#define __EXDECAYMODE_H__

#include "../FeindNs.h"
#include "Exception.h"

#include <string>

class FEIND::ExDecayMode : public Exception
{
public:
  ExDecayMode(const std::string& loc, DecayModeType mode);
};

#endif
